//
// Created by Tom Price on 31/01/2016.
//

#include "PeerConnectionClient.h"

PeerConnectionClient::PeerConnectionClient() {
    m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
    m_endpoint.clear_error_channels(websocketpp::log::elevel::all);

    m_endpoint.init_asio();
    m_endpoint.start_perpetual();

    m_endpoint.set_tls_init_handler(websocketpp::lib::bind(
            &PeerConnectionClient::on_tls_init,
            this,
            websocketpp::lib::placeholders::_1
    ));

    m_thread = websocketpp::lib::make_shared<websocketpp::lib::thread>(&ssl_client::run, &m_endpoint);
}

PeerConnectionClient::~PeerConnectionClient() {
    m_endpoint.stop_perpetual();
}

int PeerConnectionClient::connect(std::string const &uri) {
    websocketpp::lib::error_code ec;

    ssl_client::connection_ptr con = m_endpoint.get_connection(uri, ec);

    if (ec) {
        std::cerr << "> Connection initialization error: " << ec.message() << std::endl;
        return -1;
    }

    int new_id = m_next_id++;
    ConnectionMetadata::ptr metadata_ptr = websocketpp::lib::make_shared<ConnectionMetadata>(new_id, con->get_handle(),
                                                                                             uri);
    m_connection_list[new_id] = metadata_ptr;

    con->set_open_handler(websocketpp::lib::bind(
            &ConnectionMetadata::on_open,
            metadata_ptr,
            &m_endpoint,
            websocketpp::lib::placeholders::_1
    ));
    con->set_fail_handler(websocketpp::lib::bind(
            &ConnectionMetadata::on_fail,
            metadata_ptr,
            &m_endpoint,
            websocketpp::lib::placeholders::_1
    ));
    con->set_close_handler(websocketpp::lib::bind(
            &ConnectionMetadata::on_close,
            metadata_ptr,
            &m_endpoint,
            websocketpp::lib::placeholders::_1
    ));
    con->set_message_handler(websocketpp::lib::bind(
            &ConnectionMetadata::on_message,
            metadata_ptr,
            websocketpp::lib::placeholders::_1,
            websocketpp::lib::placeholders::_2
    ));

    m_endpoint.connect(con);

    return new_id;
}

void PeerConnectionClient::close(int id, websocketpp::close::status::value code, std::string reason) {
    websocketpp::lib::error_code ec;

    con_list::iterator metadata_it = m_connection_list.find(id);
    if (metadata_it == m_connection_list.end()) {
        std::cerr << "> No connection found with id " << id << std::endl;
        return;
    }

    m_endpoint.close(metadata_it->second->get_hdl(), code, reason, ec);
    if (ec) {
        std::cerr << "> Error initiating close: " << ec.message() << std::endl;
    }
}

void ConnectionMetadata::on_open(ssl_client *c, websocketpp::connection_hdl hdl) {
    m_status = OPEN;

    ssl_client::connection_ptr con = c->get_con_from_hdl(hdl);
    m_server = con->get_response_header("Server");
}

void ConnectionMetadata::on_fail(ssl_client *c, websocketpp::connection_hdl hdl) {
    m_status = FAILED;

    ssl_client::connection_ptr con = c->get_con_from_hdl(hdl);
    m_server = con->get_response_header("Server");
    m_error_reason = con->get_ec().message();
    std::cerr << "> Failed: " << m_error_reason << std::endl;
}

void ConnectionMetadata::on_close(ssl_client *c, websocketpp::connection_hdl hdl) {
    m_status = CLOSED;

    ssl_client::connection_ptr con = c->get_con_from_hdl(hdl);
    std::stringstream s;
    s << "close code: " << con->get_remote_close_code() << " ("
    << websocketpp::close::status::get_string(con->get_remote_close_code())
    << "), close reason: " << con->get_remote_close_reason();
    m_error_reason = s.str();
    std::cerr << m_error_reason;
}

void ConnectionMetadata::on_message(websocketpp::connection_hdl hdl,
                                    std::shared_ptr<websocketpp::message_buffer::message<websocketpp::message_buffer::alloc::con_msg_manager>> msg) {
    std::stringstream s;

    if (msg->get_opcode() == websocketpp::frame::opcode::text) {
        s << msg->get_payload();
    } else {
        s << websocketpp::utility::to_hex(msg->get_payload());
    }

    for(int i = 0; i < message_listeners.size(); i++) {
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

    m_endpoint.send(metadata_it->second->get_hdl(), message, websocketpp::frame::opcode::text, ec);
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
