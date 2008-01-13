#include "tinfra/io/stream.h"
#include <string>
#include <ostream>
#include <iostream>

#include <zcompat/zpio.h>

#include <unittest++/UnitTest++.h>

void write_file(const char* name, std::string const& data)
{
    tinfra::zstreambuf buf;
    buf.open_file(name, std::ios_base::out);
    {
        std::ostream o(&buf);
        o << data;
    }
    buf.close();    
}

TEST(stream_open_bad_file)
{
    tinfra::zstreambuf buf;
    CHECK_THROW( buf.open_file("this_file_doesn't_exist", std::ios_base::in), tinfra::io_exception);
}

TEST(stream_open_bad_socket)
{
    tinfra::zstreambuf buf;
    CHECK_THROW( buf.open_socket("this_host_doesnt_exist", 80), tinfra::io_exception);
}

TEST(stream_basic)
{
    write_file("a","b");
} 

TEST(stream_socket)
{
    tinfra::zstreambuf b;
    b.open_socket("google.com",80);
    {
        std::ostream o(&b);
        o << "GET / HTTP/1.0\r\n\r\n";
    }
    {
        std::istream i(&b);
        std::string t;
        while( std::getline(i, t) ) {            
            //zprintf("%s\n", t.c_str());
            std::cout << t << std::endl;
        }
    }
} 
