//
// Created by Tom Price on 31/01/2016.
//

#include "PeerConnectionClient.h"

namespace dynastat {
    PeerConnectionClient::PeerConnectionClient() {
        m_endpoint_plain.clear_access_channels(websocketpp::log::alevel::all);
        m_endpoint_plain.clear_error_channels(websocketpp::log::elevel::all);
        m_endpoint_plain.init_asio();
        m_endpoint_plain.start_perpetual();

        m_endpoint_tls.clear_access_channels(websocketpp::log::alevel::all);
        m_endpoint_tls.clear_error_channels(websocketpp::log::elevel::all);

        m_endpoint_tls.init_asio();
        m_endpoint_tls.start_perpetual();

        m_endpoint_tls.set_tls_init_handler(websocketpp::lib::bind(
                &PeerConnectionClient::on_tls_init,
                this,
                websocketpp::lib::placeholders::_1
        ));

        m_thread_plain = websocketpp::lib::make_shared<websocketpp::lib::thread>(&client_plain::run, &m_endpoint_plain);
        m_thread_tls = websocketpp::lib::make_shared<websocketpp::lib::thread>(&client_tls::run, &m_endpoint_tls);
    }

    PeerConnectionClient::~PeerConnectionClient() {
        m_endpoint_plain.stop_perpetual();
        m_endpoint_tls.stop_perpetual();

        for (con_list::const_iterator it = m_connection_list.begin(); it != m_connection_list.end(); ++it) {
            if (it->second->get_status() != ConnectionMetadata::OPEN) {
                // Only close open connections
                continue;
            }

            std::cerr << "> Closing connection " << it->second->get_id() << std::endl;

            websocketpp::lib::error_code ec;
            if (it->second->get_secure()) {
                m_endpoint_tls.close(it->second->get_hdl(), websocketpp::close::status::going_away, "", ec);
            } else {
                m_endpoint_plain.close(it->second->get_hdl(), websocketpp::close::status::going_away, "", ec);
            }
            if (ec) {
                std::cerr << "> Error closing connection " << it->second->get_id() << ": "
                << ec.message() << std::endl;
            }
        }

        m_thread_plain->join();
        m_thread_tls->join();
    }

    int PeerConnectionClient::connect(std::string const &url) {
        websocketpp::lib::error_code ec;
        websocketpp::uri *uri;
        uri = new websocketpp::uri(url);

        int new_id;

        if (!uri->get_secure()) {
            client_plain::connection_ptr con_plain = m_endpoint_plain.get_connection(url, ec);

            if (ec) {
                std::cerr << "> Connection initialization error: " << ec.message() << std::endl;
                return -1;
            }

            new_id = m_next_id++;
            ConnectionMetadata::ptr metadata_ptr = websocketpp::lib::make_shared<ConnectionMetadata>(new_id,
                                                                                                     con_plain->get_handle(),
                                                                                                     url,
                                                                                                     false);

            m_connection_list[new_id] = metadata_ptr;

            con_plain->set_open_handler(websocketpp::lib::bind(
                    &ConnectionMetadata::on_open<client_plain>,
                    metadata_ptr,
                    &m_endpoint_plain,
                    websocketpp::lib::placeholders::_1
            ));
            con_plain->set_fail_handler(websocketpp::lib::bind(
                    &ConnectionMetadata::on_fail<client_plain>,
                    metadata_ptr,
                    &m_endpoint_plain,
                    websocketpp::lib::placeholders::_1
            ));
            con_plain->set_close_handler(websocketpp::lib::bind(
                    &ConnectionMetadata::on_close<client_plain>,
                    metadata_ptr,
                    &m_endpoint_plain,
                    websocketpp::lib::placeholders::_1
            ));
            con_plain->set_message_handler(websocketpp::lib::bind(
                    &ConnectionMetadata::on_message<client_plain>,
                    metadata_ptr,
                    websocketpp::lib::placeholders::_1,
                    websocketpp::lib::placeholders::_2
            ));

            m_endpoint_plain.connect(con_plain);

        } else {
            client_tls::connection_ptr con = m_endpoint_tls.get_connection(url, ec);

            if (ec) {
                std::cerr << "> Connection initialization error: " << ec.message() << std::endl;
                return -1;
            }

            new_id = m_next_id++;
            ConnectionMetadata::ptr metadata_ptr = websocketpp::lib::make_shared<ConnectionMetadata>(new_id,
                                                                                                     con->get_handle(),
                                                                                                     url,
                                                                                                     true);

            m_connection_list[new_id] = metadata_ptr;

            con->set_open_handler(websocketpp::lib::bind(
                    &ConnectionMetadata::on_open<client_tls>,
                    metadata_ptr,
                    &m_endpoint_tls,
                    websocketpp::lib::placeholders::_1
            ));
            con->set_fail_handler(websocketpp::lib::bind(
                    &ConnectionMetadata::on_fail<client_tls>,
                    metadata_ptr,
                    &m_endpoint_tls,
                    websocketpp::lib::placeholders::_1
            ));
            con->set_close_handler(websocketpp::lib::bind(
                    &ConnectionMetadata::on_close<client_tls>,
                    metadata_ptr,
                    &m_endpoint_tls,
                    websocketpp::lib::placeholders::_1
            ));
            con->set_message_handler(websocketpp::lib::bind(
                    &ConnectionMetadata::on_message<client_tls>,
                    metadata_ptr,
                    websocketpp::lib::placeholders::_1,
                    websocketpp::lib::placeholders::_2
            ));

            m_endpoint_tls.connect(con);
        }

        return new_id;
    }

    void PeerConnectionClient::close(int id, websocketpp::close::status::value code, std::string reason) {
        websocketpp::lib::error_code ec;

        con_list::iterator metadata_it = m_connection_list.find(id);
        if (metadata_it == m_connection_list.end()) {
            std::cerr << "> No connection found with id " << id << std::endl;
            return;
        }

        if (metadata_it->second->get_secure()) {
            m_endpoint_tls.close(metadata_it->second->get_hdl(), code, reason, ec);
        } else {
            m_endpoint_plain.close(metadata_it->second->get_hdl(), code, reason, ec);
        }
        if (ec) {
            std::cerr << "> Error initiating close: " << ec.message() << std::endl;
        }
    }

    template<typename ClientType>
    void ConnectionMetadata::on_open(ClientType *c, websocketpp::connection_hdl hdl) {
        m_status = OPEN;

        typename ClientType::connection_ptr con = c->get_con_from_hdl(hdl);
        m_server = con->get_response_header("Server");
    }

    template<typename ClientType>
    void ConnectionMetadata::on_fail(ClientType *c, websocketpp::connection_hdl hdl) {
        m_status = FAILED;

        typename ClientType::connection_ptr con = c->get_con_from_hdl(hdl);
        m_server = con->get_response_header("Server");
        m_error_reason = con->get_ec().message();
        std::cerr << "> Failed: " << m_error_reason << std::endl;
    }

    template<typename ClientType>
    void ConnectionMetadata::on_close(ClientType *c, websocketpp::connection_hdl hdl) {
        m_status = CLOSED;

        typename ClientType::connection_ptr con = c->get_con_from_hdl(hdl);
        std::stringstream s;
        s << "close code: " << con->get_remote_close_code() << " ("
        << websocketpp::close::status::get_string(con->get_remote_close_code())
        << "), close reason: " << con->get_remote_close_reason();
        m_error_reason = s.str();
        std::cerr << m_error_reason;
    }

    template<typename ClientType>
    void ConnectionMetadata::on_message(websocketpp::connection_hdl hdl, typename ClientType::message_ptr msg) {
        std::stringstream s;

        if (msg->get_opcode() == websocketpp::frame::opcode::text) {
            s << msg->get_payload();
        } else {
            s << websocketpp::utility::to_hex(msg->get_payload());
        }

        for (int i = 0; i < message_listeners.size(); i++) {
            message_listeners[i]->on_message(s.str());
        }
    }

    websocketpp::connection_hdl ConnectionMetadata::get_hdl() const {
        return m_hdl;
    }

    context_ptr PeerConnectionClient::on_tls_init(websocketpp::connection_hdl) {
        context_ptr ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);

        auto options = boost::asio::ssl::context::default_workarounds | boost::asio::ssl::context::single_dh_use;
        if (!verify_ssl) {
            options = options | boost::asio::ssl::context::verify_none;
        }

        try {
            ctx->set_options(options);
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
        }

        return ctx;
    }

    void PeerConnectionClient::send(int id, std::string message) {
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
    }

    ConnectionMetadata::Status PeerConnectionClient::get_status(int id) {
        con_list::iterator metadata_it = m_connection_list.find(id);
        if (metadata_it == m_connection_list.end()) {
            std::cout << "> No connection found with id " << id << std::endl;
            return ConnectionMetadata::CLOSED;
        }

        return metadata_it->second->get_status();
    }

    void ConnectionMetadata::add_listener(PeerConnectionListener *listener) {
        message_listeners.push_back(listener);
    }

    void PeerConnectionClient::add_listener(int id, PeerConnectionListener *listener) {
        con_list::iterator metadata_it = m_connection_list.find(id);
        if (metadata_it == m_connection_list.end()) {
            std::cout << "> No connection found with id " << id << std::endl;
            return;
        }

        metadata_it->second->add_listener(listener);
    }

    int ConnectionMetadata::get_id() {
        return m_id;
    }

    bool ConnectionMetadata::get_secure() {
        return m_secure;
    }
};
