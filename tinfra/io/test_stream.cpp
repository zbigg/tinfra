#include "tinfra/io/stream.h"
#include <string>
#include <ostream>
#include <iostream>

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
        while( i ) {
            std::getline(i, t);
            std::cout << t;
        }
    }
} 
