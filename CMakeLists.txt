list(APPEND CMAKE_PREFIX_PATH "D:/Programs_it/vcpkg/installed/x64-windows-static")

find_package(unofficial-uwebsockets CONFIG REQUIRED)
find_package(unofficial-usockets CONFIG REQUIRED)

find_package(libuv REQUIRED)
find_package(ZLIB REQUIRED)

set(AXIS_SOCKETS_PUBLIC_HEADERS
        include/axis/sockets/web_socket.h
        include/axis/sockets/web_socket_manager.h
)

set(AXIS_SOCKET_SOURCES
        src/web_socket.cpp
        src/web_socket_manager.cpp
)

add_library(axis_sockets STATIC ${AXIS_SOCKET_SOURCES} ${AXIS_SOCKETS_PUBLIC_HEADERS})
add_library(axis::sockets ALIAS axis_sockets)

set_target_properties(axis_sockets PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_include_directories(axis_sockets PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_definitions(axis_sockets PUBLIC
        $<$<CONFIG:Debug>:AXIS_BUILD_DEBUG>
        $<$<CONFIG:Release>:AXIS_BUILD_RELEASE>
)

target_compile_definitions(axis_sockets PRIVATE
        AXIS_RESOURCES_DIR=\"${CMAKE_SOURCE_DIR}/resources\"
        AXIS_ROOT_DIR=\"${CMAKE_SOURCE_DIR}\"
)

target_link_libraries(axis_sockets PUBLIC
        unofficial::uwebsockets::uwebsockets
        axis::core

        ${LIBUV_LIBRARIES}
        ${ZLIB_LIBRARIES}
)

if (AXIS_BUILD_TESTS)
    add_executable(axis_sockets_tests tests/socket_test.cpp)
    target_link_libraries(axis_sockets_tests PRIVATE axis_sockets gtest gtest_main spdlog::spdlog unofficial::uwebsockets::uwebsockets)
    add_test(NAME axis_sockets_tests COMMAND axis_sockets_tests)
endif ()

