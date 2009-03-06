//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "tinfra/subprocess.h"

#include <unittest++/UnitTest++.h>

#include "tinfra/string.h"
#include <iostream>

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

SUITE(tinfra) {
    
    TEST(subprocess_spawn_no_redirect) {
        std::auto_ptr<subprocess> p = tinfra::subprocess::create();
        
        p->set_stdout_mode(subprocess::NONE);
#ifdef WIN32
        p->start("cmd /c ver");
#else
        p->start("uname -a");
#endif
        p->wait();
        CHECK_EQUAL(0, p->get_exit_code());
    }
    
    TEST(subprocess_spawn_check_exit_code) {
        std::auto_ptr<subprocess> p = tinfra::subprocess::create();
        
        p->set_stderr_mode(subprocess::NONE);
#ifdef WIN32
        p->start("cmd /c xecho hello");
#else
        p->start("uname -lollo");
#endif
        p->wait();
        CHECK(p->get_exit_code() != 0);
    }
    
    TEST(subprocess_read_stdin) {
        std::auto_ptr<subprocess> p = tinfra::subprocess::create();
                
        p->set_stdout_mode(subprocess::REDIRECT);
        std::string result;
#ifdef WIN32
        p->start("cmd /c ver");
#else
        p->start("uname");
#endif
        
        CHECK(p->get_stdin()  == 0);
        CHECK(p->get_stdout() != 0);
        CHECK(p->get_stderr() == 0);
        
        read_file(p->get_stdout(), result);        
        p->wait();
        CHECK( result.size() > 0);
        CHECK_EQUAL(0, p->get_exit_code());
    }
    
    TEST(subprocess_read_write) {
        std::string result;
        {
            std::auto_ptr<subprocess> p = tinfra::subprocess::create();
            
            p->set_stdin_mode(subprocess::REDIRECT);
            p->set_stdout_mode(subprocess::REDIRECT);
                    
            p->start("sort");
            
            CHECK(p->get_stdin()  != 0);
            CHECK(p->get_stdout() != 0);
            CHECK(p->get_stderr() == 0);
            
            write_file(p->get_stdin(), "zz\r\ncc\r\nbb\r\naa\r\n");
            p->get_stdin()->close();

            read_file(p->get_stdout(), result);
            p->wait();
        }
        using tinfra::escape_c;
        CHECK_EQUAL(escape_c("aa\r\nbb\r\ncc\r\nzz\r\n"), escape_c(result.c_str()));
    }
    
}

