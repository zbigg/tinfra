#include "tinfra/io/stream.h"
#include "tinfra/io/socket.h"
#include <ostream>
#include <istream>
#include <iostream>
#include <sstream>
#include <string>

#include <unittest++/UnitTest++.h>
#include "tinfra/test.h"

void write_file(const char* name, std::string const& data)
{
    tinfra::io::zstreambuf out;
    out.open_file(name, std::ios_base::out);
    std::stringbuf in(data);
    tinfra::io::copy(in, out);
}

void read_file(const char* name, std::string& data)
{
    tinfra::io::zstreambuf in;
    in.open_file(name, std::ios_base::in);
    std::stringbuf out;
    tinfra::io::copy(in, out);
    data = out.str();
}

SUITE(tinfra_io)
{
    TEST(open_bad_file)
    {
        tinfra::io::zstreambuf buf;
        CHECK_THROW( buf.open_file("this_file_doesn't_exist", std::ios_base::in), tinfra::io::io_exception);
    }

    TEST(open_bad_socket)
    {
        tinfra::io::zstreambuf buf;
        CHECK_THROW( buf.open_socket("this_host_doesnt_exist", 80), tinfra::io::io_exception);
    }

    TEST(basic)
    {
        const char* text = "abc \ndef \r\ndef\a\t\xafgef";
        tinfra::test::TempTestLocation testLocation;
        write_file("a",text);
        std::string b;
        read_file("a", b);
        CHECK_EQUAL(text, b);
    } 
}
