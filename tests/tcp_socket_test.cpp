//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/tcp_socket.h"

#include "tinfra/thread.h"

#include "tinfra/test.h" // for test infra

SUITE(tinfra) {
    using tinfra::tcp_client_socket;
    using tinfra::tcp_server_socket;

    struct test_server {
        tcp_server_socket& server;

        void operator()() {
            std::string address;
            std::auto_ptr<tcp_client_socket> client = server.accept(address);
            char c;
            CHECK_EQUAL(1, client->read(&c, 1));
            CHECK_EQUAL(1, client->write(&c, 1));
            client->close();
        }
    };
    
    TEST(tcp_socket_serve_and_connect)
    {
        tcp_server_socket server_socket("", 10999);
        test_server server = { server_socket };
        tinfra::thread::thread t = tinfra::thread::thread::start(server);
        {
            tcp_client_socket client("localhost", 10999);
            char in = 0xff;
            char out = 2;
            CHECK_EQUAL(1, client.write(&in, 1));
            CHECK_EQUAL(1, client.read(&out, 1));
            CHECK_EQUAL(int(in), int(out));
        }
        t.join();
    }
}

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:



