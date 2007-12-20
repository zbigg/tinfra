#include "tinfra/io/stream.h"
#include <string>
#include <ostream>

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
