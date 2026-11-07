#include "axis/sockets/web_socket_manager.h"

SocketManager::WebSocketVariant SocketManager::openSocket(int port_name)
{
    if (!axis::core::Logger::getInstance().getSpdlogLogger())
    {
        axis::core::Logger::getInstance().init();
    }


    auto& ex = axis::core::Config::getInstance();
    auto loadRes = ex.loadFile(AXIS_RESOURCES_DIR "/config_server.ini");

    bool flag = false;

    if (loadRes) {
        auto& config = axis::core::Config::getInstance();

        auto flag_result = config.getBool("SSL", "ssl_socket", false);

        if (flag_result.isOk())
        {
            flag = flag_result.value();
            axis::core::Logger::getInstance().getSpdlogLogger()->info("FIND FLAG");
            axis::core::Logger::getInstance().getSpdlogLogger()->info(flag);
        }
    }

    try
    {
        if (flag)
        {
            auto new_socket = std::make_shared<WebSocket<true>>(port_name);
            if (new_socket)
            {
                sockets[port_name] = new_socket;
                return new_socket;
            }
            axis::core::Logger::getInstance().getSpdlogLogger()->info("[WebSocket] Error with creating socket");
        }
        else
        {
            auto new_socket = std::make_shared<WebSocket<false>>(port_name);
            if (new_socket)
            {
                sockets[port_name] = new_socket;
                return new_socket;
            }
            axis::core::Logger::getInstance().getSpdlogLogger()->info("[WebSocket] Error with creating socket");
        }
    }
    catch (...)
    {
        axis::core::Logger::getInstance().getSpdlogLogger()->error("[WebSocket] Exception creating socket");
    }

    return std::monostate{};
}

void SocketManager::startListening(int port_name)
{
    auto cur_socket = this->sockets.find(port_name);
    if (cur_socket != this->sockets.end())
    {
        std::visit(
            [](auto& w)
            {
                using T = std::decay_t<decltype(w)>;
                if constexpr (!std::is_same_v<T, std::monostate>)
                {
                    if (w)
                    {
                        w->startListening();
                    }
                }
            },
            cur_socket->second);
    }
}

void SocketManager::closeSocket(int port_name)
{
    auto cur_socket = this->sockets.find(port_name);
    if (cur_socket != this->sockets.end())
    {
        std::visit(
            [](auto& w)
            {
                using T = std::decay_t<decltype(w)>;
                if constexpr (!std::is_same_v<T, std::monostate>)
                {
                    if (w)
                    {
                        w->closeSocket();
                    }
                }
            },
            cur_socket->second);
    }
}

void SocketManager::closeAllSockets()
{
    for (auto sock : this->sockets)
    {
        this->closeSocket(sock.first);
    }
    this->sockets.clear();
}

void SocketManager::closeManager()
{
    this->closeAllSockets();
}
