#include <gtest/gtest.h>


//ТЕСТЫ ДЛЯ БИБЛИОТЕКИ

//[SOCKET_TESTS]
//данные конфигурация
//проверка создания сокета
//проверка "слушания сокета"
//деструктор сокета

//[SOCKET_MANAGER_TESTS]
//менеджер - инициализация поля map
//менеджер - деструктор

#include "axis/sockets/web_socket.h"
#include "axis/sockets/web_socket_manager.h"

#define TEST_PORT 9000

namespace
{

    TEST(sockets_tests, initialization_test) {
        auto socket_manager = std::make_shared<SocketManager>();
        auto socket = socket_manager->openSocket(TEST_PORT);

        ASSERT_FALSE(socket.valueless_by_exception()) << "Incorrect option return from the constructor";

        int res = -1;
        std::visit([&res](auto& w) { res = w->getPort(); }, socket);
        ASSERT_EQ(TEST_PORT, res) << "Incorrect port number during initialization";

        socket_manager->close_socket(TEST_PORT);
        socket_manager.reset();
    }

    /*TEST(sockets_tests, destructor_socketapp_test) {
        auto socket_manager = std::make_shared<SocketManager>();
        bool executed = false;
        {
            std::shared_ptr<WebSocket<false>> socket(new WebSocket<false>(TEST_PORT), [&executed](WebSocket<false>*) { executed = true; });
        }

        socket_manager.reset();

        ASSERT_TRUE(executed) << "Incorrect destruction socket app";
    }*/

    /*TEST(sockets_tests, destructor_socketSSLapp_test) {
        bool executed = false;
        {
            std::shared_ptr<WebSocket<true>> socket(new WebSocket<true>(TEST_PORT), [&executed](WebSocket<true>*) { executed = true; });
        }

        ASSERT_TRUE(executed) << "Incorrect destruction ssl-socket app";
    }*/

    TEST(SOCKET_TESTS, ssl_flag_test)
    {
        auto socket_manager = std::make_shared<SocketManager>();
        auto socket = socket_manager->openSocket(TEST_PORT);

        bool res;
        std::visit([&res](auto& w) {
            res = w->getType();
        }, socket);

        socket_manager->close_socket(TEST_PORT);
        socket_manager.reset();

        if (std::strcmp(SSL_SOCKET, "ON") == 0) {
            ASSERT_TRUE(res) << "Socket[wss] was opened with no SSL";
        } else {
            ASSERT_FALSE(res) << "Socket[ws] was opened with SSL";
        }
    }
}