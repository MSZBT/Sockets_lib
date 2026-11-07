#include "axis/sockets/web_socket.h"

#ifndef AXIS_RESOURCES_DIR
#define AXIS_RESOURCES_DIR "."
#endif

template<bool SSL_FLAG>
WebSocket<SSL_FLAG>::WebSocket(int port)
    : port(port)
{
    getSocketConfiguration();
    getSocketHandlers();

    if (!axis::core::Logger::getInstance().getSpdlogLogger())
    {
        axis::core::Logger::getInstance().init();
    }

    if constexpr (SSL_FLAG)
    {
        if (!getSSLConfiguration())
        {
            axis::core::Logger::getInstance().getSpdlogLogger()->info("Cannot open socket with empty ssl properties");
        }
        app = std::make_unique<AppType>(ssl_conf);
    }
    else
    {
        app = std::make_unique<AppType>();
    }

    app->ws("/*", std::move(behavior))
        .listen(
            this->getPort(),
            [this](auto* listen_socket)
            {
                if (listen_socket)
                {
                    this->listening = true;
                    axis::core::Logger::getInstance().getSpdlogLogger()->info(
                        std::format("[WebSocket] {} start listening", this->getPort()));
                }
                else
                {
                    axis::core::Logger::getInstance().getSpdlogLogger()->info(
                        std::format("[WebSocket] {} stop listening", this->getPort()));

                    throw std::logic_error("NO SSL");
                }
            });
}

template<bool SSL_FLAG>
void WebSocket<SSL_FLAG>::getSocketConfiguration()
{
    if (!axis::core::Logger::getInstance().getSpdlogLogger())
    {
        axis::core::Logger::getInstance().init();
    }

    auto& ex = axis::core::Config::getInstance();

    if (auto loadRes = ex.loadFile(AXIS_RESOURCES_DIR "/config_server.ini"))
    {
        auto& config = axis::core::Config::getInstance();
        axis::core::Logger::getInstance().getSpdlogLogger()->info("[WebSocket] get config");

        auto maxPayloadLength= config.getInt("Sockets", "maxPayloadLength");
        if (maxPayloadLength.isOk()) {
            this->behavior.maxPayloadLength = maxPayloadLength.value();
        }

        auto idleTimeout_result = config.getInt("Sockets", "idleTimeout");
        if (idleTimeout_result.isOk()) {
            this->behavior.idleTimeout = idleTimeout_result.value();
        }

        auto maxBackpressure_result = config.getInt("Sockets", "maxBackpressure");
        if (maxBackpressure_result.isOk()) {
            this->behavior.maxBackpressure = maxBackpressure_result.value();
        }

        auto closeOnBackpressureLimit_result = config.getInt("Sockets", "closeOnBackpressureLimit");
        if (closeOnBackpressureLimit_result.isOk()) {
            this->behavior.closeOnBackpressureLimit = closeOnBackpressureLimit_result.value();
        }

        auto resetIdleTimeoutOnSend_result = config.getBool("Sockets", "resetIdleTimeoutOnSend");
        if (resetIdleTimeoutOnSend_result.isOk()) {
            this->behavior.resetIdleTimeoutOnSend = resetIdleTimeoutOnSend_result.value();
        }

        auto sendPingsAutomatically_result = config.getBool("Sockets", "sendPingsAutomatically");
        if (sendPingsAutomatically_result.isOk()) {
            this->behavior.sendPingsAutomatically = sendPingsAutomatically_result.value();
        }
    }

    else
    {
        axis::core::Logger::getInstance().getSpdlogLogger()->info(std::format("Error with config parsing {}", this->getPort()));
    }
}

template<bool SSL_FLAG>
void WebSocket<SSL_FLAG>::getSocketHandlers()
{
    this->behavior.upgrade = nullptr;

    this->behavior.open = [this](auto* /*ws*/)
    {
        axis::core::Logger::getInstance().getSpdlogLogger()->info(std::format("[WebSocket] open {}", this->getPort()));
    };

    this->behavior.message = [this](auto* ws, std::string_view message, uWS::OpCode opCode)
    {
        axis::core::Logger::getInstance().getSpdlogLogger()->info(std::format("[WebSocket] get message {}: {}", this->getPort(), message));
        axis::core::Logger::getInstance().getSpdlogLogger()->info(std::format("[WebSocket] send message {}: Pong", this->getPort()));
        ws->send("Pong", uWS::OpCode::TEXT);
    };

    this->behavior.dropped = [](auto* /*ws*/, std::string_view /*message*/, uWS::OpCode /*opCode*/)
    {
    };

    this->behavior.drain = [](auto* /*ws*/)
    {
    };

    this->behavior.ping = [](auto* /*ws*/, std::string_view)
    {
    };

    this->behavior.pong = [](auto* /*ws*/, std::string_view)
    {
    };

    this->behavior.close = [this](auto* /*ws*/, int /*code*/, std::string_view /*message*/)
    {
        this->listening = false;
        axis::core::Logger::getInstance().getSpdlogLogger()->info(std::format("[WebSocket] closed {} ", this->getPort()));
    };
}

template<bool SSL_FLAG>
bool WebSocket<SSL_FLAG>::getSSLConfiguration()
{
    if (!axis::core::Logger::getInstance().getSpdlogLogger())
    {
        axis::core::Logger::getInstance().init();
    }

    auto& ex = axis::core::Config::getInstance();

    if (auto loadRes = ex.loadFile(AXIS_RESOURCES_DIR "/config_server.ini"))
    {
        axis::core::Logger::getInstance().getSpdlogLogger()->info("[WebSocket] Trying to get SSL configs");

        auto& config = axis::core::Config::getInstance();

        auto key_file_name_result = config.getString("SSL", "key_path");
        if (key_file_name_result.isOk())
        {
            std::string key_file_name_path = std::string(AXIS_ROOT_DIR) + "/" + key_file_name_result.value();

            this->key_file_name_tmp = key_file_name_path;
            this->ssl_conf.key_file_name = this->key_file_name_tmp.c_str();
        }
        else
        {
            axis::core::Logger::getInstance().getSpdlogLogger()->info("[WebSocket] No ssl properties in config");
            return false;
        }

        auto cert_file_name_result = config.getString("SSL", "cert_path");
        if (cert_file_name_result.isOk())
        {
            std::string cert_file_name_path = std::string(AXIS_ROOT_DIR) + "/" + cert_file_name_result.value();

            this->cert_file_name_tmp = cert_file_name_path;
            this->ssl_conf.cert_file_name = this->cert_file_name_tmp.c_str();
        }
        else
        {
            axis::core::Logger::getInstance().getSpdlogLogger()->info("[WebSocket] No ssl properties in config");
            return false;
        }

        auto passphrase_result = config.getString("SSL", "passphrase");
        if (passphrase_result.isOk())
        {
            this->passphrase_tmp = passphrase_result.value();
            this->ssl_conf.passphrase = passphrase_tmp.c_str();
        }
        else
        {
            axis::core::Logger::getInstance().getSpdlogLogger()->info("[WebSocket] No ssl properties in config");
            return false;
        }
        return true;
    }
    else
    {
        axis::core::Logger::getInstance().getSpdlogLogger()->info(
            std::format("[WebSocket] Error with find ssl config parsing {}", this->getPort()));
        return false;
    }
}

template<bool SSL_FLAG>
void WebSocket<SSL_FLAG>::startListening()
{
    this->app->run();
}

template<bool SSL_FLAG>
void WebSocket<SSL_FLAG>::closeSocket()
{
    this->app->close();
}

template class WebSocket<true>;
template class WebSocket<false>;
