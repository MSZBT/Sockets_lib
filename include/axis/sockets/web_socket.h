#ifndef AXIS_WEB_SOCKET_H
#define AXIS_WEB_SOCKET_H

#ifndef AXIS_ROOT_DIR
#define AXIS_ROOT_DIR "."
#endif

#include "axis/core/config.h"
#include "axis/core/logger.h"

#include <uwebsockets/App.h>

#include <format>
#include <type_traits>

template<bool SSL_FLAG>
class WebSocket
{
private:
    using AppType = std::conditional_t<SSL_FLAG, uWS::SSLApp, uWS::App>;
    std::unique_ptr<AppType> app;

    int port;
    bool listening = false;

public:

    bool ssl_flag = SSL_FLAG;
    explicit WebSocket(int port);
    ~WebSocket() = default;

    struct UserData
    {
    };

    typename AppType::template WebSocketBehavior<UserData> behavior = { .compression = uWS::CompressOptions::DEDICATED_COMPRESSOR };

    std::string key_file_name_tmp;
    std::string cert_file_name_tmp;
    std::string passphrase_tmp;

    uWS::SocketContextOptions ssl_conf{
        .key_file_name = "/default",
        .cert_file_name = "/default",
        .passphrase = "/default",
    };

    void getSocketConfiguration();
    void getSocketHandlers();
    bool getSSLConfiguration();

    void startListening();
    void closeSocket();

    bool getType()
    {
        return this->ssl_flag;
    }

    int getPort()
    {
        return this->port;
    }

    bool getListening()
    {
        return this->listening;
    }
};

#endif // AXIS_WEB_SOCKET_H