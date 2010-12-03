//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/subprocess.h"

#include "tinfra/fmt.h"

#include <unittest++/UnitTest++.h>

#include "tinfra/string.h"
#include "tinfra/tstring.h"
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

    using tinfra::tstring;
    
    /// returns command string that echoes a env variable
    static std::string get_echo_var_command(tstring const& varname)
    {
#ifdef _WIN32
        return tinfra::fmt("cmd /c \"echo %%%s%%\"") % varname;
#else
        return tinfra::fmt("echo $%s") % varname;
#endif
    }
    
    /// returns expected output of echo $VAR command with variable
    /// that is unknown
    static std::string unknown_variable_output(tstring const& name)
    {
#ifdef _WIN32
            return tinfra::fmt("%%%s%%") % name;
#else
            return "";
#endif
    }
    
    TEST(subprocess_sets_environment)
    {
        using tinfra::environment_t;
        using tinfra::fmt;
        using tinfra::capture_command;
        using tinfra::strip;
        
        environment_t base_env = tinfra::get_environment();
        std::string unique_name = "thiSSurelyAkukuNotExists";
        {
            
            CHECK( base_env.find(unique_name) == base_env.end());
            
            environment_t modified_env(base_env);
            modified_env[unique_name] = unique_name;
            std::string command = get_echo_var_command(unique_name);
            
            // check that subshell has our variable in modified env
            CHECK_EQUAL( unique_name, strip(capture_command(command, modified_env)) );

            // check that subshell don't see our variable in base env
            CHECK_EQUAL( unknown_variable_output(unique_name), strip(capture_command(command, base_env)) );
        }
    }
    TEST(subprocess_clears_environment) {
        
        using tinfra::environment_t;
        using tinfra::capture_command;
        using tinfra::strip;
        
        environment_t base_env = tinfra::get_environment();
        {
#ifdef WIN32
            std::string erased_variable = "SYSTEMROOT";
#else
            std::string erased_variable = "HOME";
#endif      
            CHECK( base_env.find(erased_variable) != base_env.end());
            environment_t modified_env(base_env);
            modified_env.erase(erased_variable);
            
            std::string command = get_echo_var_command(erased_variable);
            CHECK_EQUAL( unknown_variable_output(erased_variable), strip(capture_command(command, modified_env)) );
        }
    }
}

