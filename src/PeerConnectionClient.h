//
// Created by Tom Price on 31/01/2016.
//

#ifndef CDYNASTAT_PEERCONNECTIONCLIENT_H
#define CDYNASTAT_PEERCONNECTIONCLIENT_H

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>

typedef websocketpp::client<websocketpp::config::asio_tls_client> ssl_client;
typedef websocketpp::lib::shared_ptr<boost::asio::ssl::context> context_ptr;

class PeerConnectionListener {
public:
    virtual void on_message(std::string msg) = 0;
};

class ConnectionMetadata {
public:
    typedef websocketpp::lib::shared_ptr<ConnectionMetadata> ptr;

    enum Status {
        CONNECTING,
        OPEN,
        FAILED,
        CLOSED
    };

    ConnectionMetadata(int id, websocketpp::connection_hdl hdl, std::string uri) :
            m_id(id),
            m_hdl(hdl),
            m_status(CONNECTING),
            m_uri(uri) { };

    void on_open(ssl_client *c, websocketpp::connection_hdl hdl);

    void on_fail(ssl_client *c, websocketpp::connection_hdl hdl);

    void on_close(ssl_client *c, websocketpp::connection_hdl hdl);

    void on_message(websocketpp::connection_hdl hdl, ssl_client::message_ptr msg);

    const Status &get_status() {
        return m_status;
    }

    websocketpp::connection_hdl get_hdl() const;

    void add_listener(PeerConnectionListener *listener);

private:
    int m_id;
    websocketpp::connection_hdl m_hdl;
    Status m_status;
    std::string m_uri;
    std::string m_server;
    std::string m_error_reason;

    std::vector<PeerConnectionListener*> message_listeners;
};

class PeerConnectionClient {
public:
    PeerConnectionClient();

    ~PeerConnectionClient();

    context_ptr on_tls_init(websocketpp::connection_hdl);

    int connect(std::string const &uri);

    void close(int id, websocketpp::close::status::value code, std::string reason);

    void send(int id, std::string msg);

    void add_listener(int id, PeerConnectionListener *listener);

    ConnectionMetadata::Status get_status(int id);

private:
    typedef std::map<int, ConnectionMetadata::ptr> con_list;

    ssl_client m_endpoint;
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;

    con_list m_connection_list;
    int m_next_id = 0;
    const bool verify_ssl = false;
};


#endif //CDYNASTAT_PEERCONNECTIONCLIENT_H
