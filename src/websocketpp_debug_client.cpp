/*
 * Copyright (c) 2014, Peter Thorson. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the WebSocket++ Project nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL PETER THORSON BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// **NOTE:** This file is a snapshot of the WebSocket++ utility client tutorial.
// Additional related material can be found in the tutorials/utility_client
// directory of the WebSocket++ repository.

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>

#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <sstream>

typedef websocketpp::client<websocketpp::config::asio_client> client_plain;
typedef websocketpp::client<websocketpp::config::asio_tls_client> client_tls;

class connection_metadata {
public:
    typedef websocketpp::lib::shared_ptr<connection_metadata> ptr;

    connection_metadata(int id, websocketpp::connection_hdl hdl, std::string uri, bool secure = true)
            : m_id(id), m_hdl(hdl), m_status("Connecting"), m_uri(uri), m_server("N/A"), m_secure(secure) { }

    template<typename ClientType>
    void on_open(ClientType *c, websocketpp::connection_hdl hdl) {
        m_status = "Open";

        typename ClientType::connection_ptr con = c->get_con_from_hdl(hdl);
        m_server = con->get_response_header("Server");
    }

    template<typename ClientType>
    void on_fail(ClientType *c, websocketpp::connection_hdl hdl) {
        m_status = "Failed";

        typename ClientType::connection_ptr con = c->get_con_from_hdl(hdl);
        m_server = con->get_response_header("Server");
        m_error_reason = con->get_ec().message();
    }

    template<typename ClientType>
    void on_close(ClientType *c, websocketpp::connection_hdl hdl) {
        m_status = "Closed";
        typename ClientType::connection_ptr con = c->get_con_from_hdl(hdl);
        std::stringstream s;
        s << "close code: " << con->get_remote_close_code() << " ("
        << websocketpp::close::status::get_string(con->get_remote_close_code())
        << "), close reason: " << con->get_remote_close_reason();
        m_error_reason = s.str();
    }

    template<typename ClientType>
    void on_message(websocketpp::connection_hdl, typename ClientType::message_ptr msg) {
        if (msg->get_opcode() == websocketpp::frame::opcode::text) {
            m_messages.push_back("<< " + msg->get_payload());
            std::cout << "Recieved: " << msg->get_payload() << std::endl;
        } else {
            m_messages.push_back("<< " + websocketpp::utility::to_hex(msg->get_payload()));
        }
    }

    websocketpp::connection_hdl get_hdl() const {
        return m_hdl;
    }

    int get_id() const {
        return m_id;
    }

    std::string get_status() const {
        return m_status;
    }

    void record_sent_message(std::string message) {
        m_messages.push_back(">> " + message);
    }

    friend std::ostream &operator<<(std::ostream &out, connection_metadata const &data);

    bool get_secure() {
        return m_secure;
    }

private:
    int m_id;
    websocketpp::connection_hdl m_hdl;
    std::string m_status;
    std::string m_uri;
    std::string m_server;
    std::string m_error_reason;
    std::vector<std::string> m_messages;
    bool m_secure;
};

std::ostream &operator<<(std::ostream &out, connection_metadata const &data) {
    out << "> URI: " << data.m_uri << "\n"
    << "> Status: " << data.m_status << "\n"
    << "> Remote Server: " << (data.m_server.empty() ? "None Specified" : data.m_server) << "\n"
    << "> Error/close reason: " << (data.m_error_reason.empty() ? "N/A" : data.m_error_reason) << "\n";
    out << "> Messages Processed: (" << data.m_messages.size() << ") \n";

    std::vector<std::string>::const_iterator it;
    for (it = data.m_messages.begin(); it != data.m_messages.end(); ++it) {
        out << *it << "\n";
    }

    return out;
}

class websocket_endpoint {
public:
    websocket_endpoint() : m_next_id(0) {
        m_endpoint_plain.clear_access_channels(websocketpp::log::alevel::all);
        m_endpoint_plain.clear_error_channels(websocketpp::log::elevel::all);

        m_endpoint_plain.init_asio();
        m_endpoint_plain.start_perpetual();

        m_endpoint_tls.clear_access_channels(websocketpp::log::alevel::all);
        m_endpoint_tls.clear_error_channels(websocketpp::log::elevel::all);

        m_endpoint_tls.set_tls_init_handler([this](websocketpp::connection_hdl) {
            return websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context_base::sslv23);
        });

        m_endpoint_tls.init_asio();
        m_endpoint_tls.start_perpetual();

        m_thread_plain = websocketpp::lib::make_shared<websocketpp::lib::thread>(&client_plain::run, &m_endpoint_plain);
        m_thread_tls = websocketpp::lib::make_shared<websocketpp::lib::thread>(&client_tls::run, &m_endpoint_tls);
    }

    ~websocket_endpoint() {
        m_endpoint_plain.stop_perpetual();
        m_endpoint_tls.stop_perpetual();

        for (con_list::const_iterator it = m_connection_list.begin(); it != m_connection_list.end(); ++it) {
            if (it->second->get_status() != "Open") {
                // Only close open connections
                continue;
            }

            std::cout << "> Closing connection " << it->second->get_id() << std::endl;

            websocketpp::lib::error_code ec;
            if (it->second->get_secure()) {
                m_endpoint_tls.close(it->second->get_hdl(), websocketpp::close::status::going_away, "", ec);
            } else {
                m_endpoint_plain.close(it->second->get_hdl(), websocketpp::close::status::going_away, "", ec);
            }
            if (ec) {
                std::cout << "> Error closing connection " << it->second->get_id() << ": "
                << ec.message() << std::endl;
            }
        }

        m_thread_plain->join();
        m_thread_tls->join();
    }

    int connect(std::string const &url) {
        websocketpp::lib::error_code ec;
        websocketpp::uri *uri = new websocketpp::uri(url);
        int new_id;

        if (uri->get_secure()) {
            client_tls::connection_ptr con = m_endpoint_tls.get_connection(url, ec);

            if (ec) {
                std::cout << "> Connect initialization error: " << ec.message() << std::endl;
                return -1;
            }

            new_id = m_next_id++;
            connection_metadata::ptr metadata_ptr = websocketpp::lib::make_shared<connection_metadata>(new_id,
                                                                                                       con->get_handle(),
                                                                                                       url,
                                                                                                       true);

            m_connection_list[new_id] = metadata_ptr;

            con->set_open_handler(websocketpp::lib::bind(
                    &connection_metadata::on_open<client_tls>,
                    metadata_ptr,
                    &m_endpoint_tls,
                    websocketpp::lib::placeholders::_1
            ));
            con->set_fail_handler(websocketpp::lib::bind(
                    &connection_metadata::on_fail<client_tls>,
                    metadata_ptr,
                    &m_endpoint_tls,
                    websocketpp::lib::placeholders::_1
            ));
            con->set_close_handler(websocketpp::lib::bind(
                    &connection_metadata::on_close<client_tls>,
                    metadata_ptr,
                    &m_endpoint_tls,
                    websocketpp::lib::placeholders::_1
            ));
            con->set_message_handler(websocketpp::lib::bind(
                    &connection_metadata::on_message<client_tls>,
                    metadata_ptr,
                    websocketpp::lib::placeholders::_1,
                    websocketpp::lib::placeholders::_2
            ));

            m_endpoint_tls.connect(con);
        } else {
            client_plain::connection_ptr con = m_endpoint_plain.get_connection(url, ec);

            if (ec) {
                std::cout << "> Connect initialization error: " << ec.message() << std::endl;
                return -1;
            }

            new_id = m_next_id++;
            connection_metadata::ptr metadata_ptr = websocketpp::lib::make_shared<connection_metadata>(new_id,
                                                                                                       con->get_handle(),
                                                                                                       url,
                                                                                                       false);

            m_connection_list[new_id] = metadata_ptr;

            con->set_open_handler(websocketpp::lib::bind(
                    &connection_metadata::on_open<client_plain>,
                    metadata_ptr,
                    &m_endpoint_plain,
                    websocketpp::lib::placeholders::_1
            ));
            con->set_fail_handler(websocketpp::lib::bind(
                    &connection_metadata::on_fail<client_plain>,
                    metadata_ptr,
                    &m_endpoint_plain,
                    websocketpp::lib::placeholders::_1
            ));
            con->set_close_handler(websocketpp::lib::bind(
                    &connection_metadata::on_close<client_plain>,
                    metadata_ptr,
                    &m_endpoint_plain,
                    websocketpp::lib::placeholders::_1
            ));
            con->set_message_handler(websocketpp::lib::bind(
                    &connection_metadata::on_message<client_plain>,
                    metadata_ptr,
                    websocketpp::lib::placeholders::_1,
                    websocketpp::lib::placeholders::_2
            ));

            m_endpoint_plain.connect(con);
        }

        return new_id;
    }

    void close(int id, websocketpp::close::status::value code, std::string reason) {
        websocketpp::lib::error_code ec;

        con_list::iterator metadata_it = m_connection_list.find(id);
        if (metadata_it == m_connection_list.end()) {
            std::cout << "> No connection found with id " << id << std::endl;
            return;
        }

        if (metadata_it->second->get_secure()) {
            m_endpoint_tls.close(metadata_it->second->get_hdl(), code, reason, ec);
        } else {
            m_endpoint_plain.close(metadata_it->second->get_hdl(), code, reason, ec);
        }
        if (ec) {
            std::cout << "> Error initiating close: " << ec.message() << std::endl;
        }
    }

    void send(int id, std::string message) {
        websocketpp::lib::error_code ec;

        con_list::iterator metadata_it = m_connection_list.find(id);
        if (metadata_it == m_connection_list.end()) {
            std::cout << "> No connection found with id " << id << std::endl;
            return;
        }

        if (metadata_it->second->get_secure()) {
            m_endpoint_tls.send(metadata_it->second->get_hdl(), message, websocketpp::frame::opcode::text, ec);
        } else {
            m_endpoint_plain.send(metadata_it->second->get_hdl(), message, websocketpp::frame::opcode::text, ec);
        }
        if (ec) {
            std::cout << "> Error sending message: " << ec.message() << std::endl;
            return;
        }

        metadata_it->second->record_sent_message(message);
    }

    connection_metadata::ptr get_metadata(int id) const {
        con_list::const_iterator metadata_it = m_connection_list.find(id);
        if (metadata_it == m_connection_list.end()) {
            return connection_metadata::ptr();
        } else {
            return metadata_it->second;
        }
    }

private:
    typedef std::map<int, connection_metadata::ptr> con_list;

    client_tls m_endpoint_tls;
    client_plain m_endpoint_plain;
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread_plain;
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread_tls;

    con_list m_connection_list;
    int m_next_id;
};

int main() {
    bool done = false;
    std::string input;
    websocket_endpoint endpoint;

    while (!done) {
        std::cout << "Enter Command: ";
        std::getline(std::cin, input);

        if (input == "quit") {
            done = true;
        } else if (input == "help") {
            std::cout
            << "\nCommand List:\n"
            << "connect <ws uri>\n"
            << "send <connection id> <message>\n"
            << "close <connection id> [<close code:default=1000>] [<close reason>]\n"
            << "show <connection id>\n"
            << "help: Display this help text\n"
            << "quit: Exit the program\n"
            << std::endl;
        } else if (input.substr(0, 7) == "connect") {
            int id = endpoint.connect(input.substr(8));
            if (id != -1) {
                std::cout << "> Created connection with id " << id << std::endl;
            }
        } else if (input.substr(0, 4) == "send") {
            std::stringstream ss(input);

            std::string cmd;
            int id;
            std::string message = "";

            ss >> cmd >> id;
            std::getline(ss, message);

            endpoint.send(id, message);
        } else if (input.substr(0, 5) == "close") {
            std::stringstream ss(input);

            std::string cmd;
            int id;
            int close_code = websocketpp::close::status::normal;
            std::string reason = "";

            ss >> cmd >> id >> close_code;
            std::getline(ss, reason);

            endpoint.close(id, close_code, reason);
        } else if (input.substr(0, 4) == "show") {
            int id = atoi(input.substr(5).c_str());

            connection_metadata::ptr metadata = endpoint.get_metadata(id);
            if (metadata) {
                std::cout << *metadata << std::endl;
            } else {
                std::cout << "> Unknown connection id " << id << std::endl;
            }
        } else {
            std::cout << "> Unrecognized Command" << std::endl;
        }
    }

    return 0;
}

/*

clang++ -std=c++11 -stdlib=libc++ -I/Users/zaphoyd/software/websocketpp/ -I/Users/zaphoyd/software/boost_1_55_0/ -D_WEBSOCKETPP_CPP11_STL_ step4.cpp /Users/zaphoyd/software/boost_1_55_0/stage/lib/libboost_system.a

clang++ -I/Users/zaphoyd/software/websocketpp/ -I/Users/zaphoyd/software/boost_1_55_0/ step4.cpp /Users/zaphoyd/software/boost_1_55_0/stage/lib/libboost_system.a /Users/zaphoyd/software/boost_1_55_0/stage/lib/libboost_thread.a /Users/zaphoyd/software/boost_1_55_0/stage/lib/libboost_random.a

clang++ -std=c++11 -stdlib=libc++ -I/Users/zaphoyd/Documents/websocketpp/ -I/Users/zaphoyd/Documents/boost_1_53_0_libcpp/ -D_WEBSOCKETPP_CPP11_STL_ step4.cpp /Users/zaphoyd/Documents/boost_1_53_0_libcpp/stage/lib/libboost_system.a

*/
