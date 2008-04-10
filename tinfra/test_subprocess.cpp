#include "tinfra/subprocess.h"

#include <unittest++/UnitTest++.h>

using tinfra::subprocess;
using tinfra::io::stream;

static void write_file(stream* s, std::string const& data)
{
    s->write(data.c_str(), data.size());
}

static void read_file(stream* s, std::string& data)
{
    char buf[1024];
    int r;
    while( ( r = s->read(buf, sizeof(buf))) > 0 ) {
        data.append(buf, r);
    }
}

SUITE(tinfra_subprocess) {
    
    TEST(simple) {
        std::auto_ptr<subprocess> p(tinfra::create_subprocess());
        
        p->set_stdin_mode(subprocess::REDIRECT);
        p->set_stdout_mode(subprocess::REDIRECT);        
        
        p->start("sort");
        write_file(p->get_stdin(), "z\nc\nb\na\n");
        p->get_stdin()->close();

        std::string result;
        read_file(p->get_stdout(), result);
        p->wait();
        CHECK_EQUAL("a\nb\nc\nz\n", result.c_str());
    }
    
}

