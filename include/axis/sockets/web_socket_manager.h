#ifndef AXIS_WEB_SOCKET_MANAGER_H
#define AXIS_WEB_SOCKET_MANAGER_H

#include "axis/sockets/web_socket.h"
#include "uwebsockets/App.h"

#include <map>
#include <memory>
#include <variant>

class SocketManager
{
private:
    using WebSocketVariant = std::variant<std::monostate, std::shared_ptr<WebSocket<true>>, std::shared_ptr<WebSocket<false>>>;
    std::map<int, WebSocketVariant> sockets;

public:
    SocketManager() = default;

    WebSocketVariant openSocket(int port_name);
    void startListening(int port_name);
    void closeSocket(int port_name);
    void closeManager();
    void closeAllSockets();

    WebSocketVariant getSocket(int port)
    {
        return sockets[port];
    };
};
#endif // AXIS_WEB_SOCKET_MANAGER_H