//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "tinfra/io/stream.h"
#include "tinfra/io/socket.h"
#include <ostream>
#include <istream>
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>

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

SUITE(tinfra)
{
    TEST(io_open_bad_file)
    {
        tinfra::io::zstreambuf buf;
        CHECK_THROW( buf.open_file("this_file_doesn't_exist", std::ios_base::in), std::logic_error);
    }

    TEST(io_open_bad_socket)
    {
        tinfra::io::zstreambuf buf;
        CHECK_THROW( buf.open_socket("this_host_doesnt_exist", 80), std::logic_error);
    }

    TEST(io_basic)
    {
        const char* text = "abc \ndef \r\ndef\a\t\xafgef";
        tinfra::test::TempTestLocation testLocation;
        write_file("a",text);
        std::string b;
        read_file("a", b);
        CHECK_EQUAL(text, b);
    } 
}
