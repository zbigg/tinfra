//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/subprocess.h" // API under test

#include "tinfra/fmt.h"

#include "tinfra/test.h" // test infra

#include "tinfra/string.h"
#include "tinfra/stream.h" // for tinfra::read_all, write_all
#include "tinfra/tstring.h"
#include <iostream>

using tinfra::subprocess;
using tinfra::io::stream;

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
        using tinfra::read_all;
        using tinfra::write_all;
        
        std::auto_ptr<subprocess> p = tinfra::subprocess::create();
                
        p->set_stdout_mode(subprocess::REDIRECT);
        
#ifdef WIN32
        p->start("cmd /c ver");
#else
        p->start("uname");
#endif
        
        CHECK(p->get_stdin()  == 0);
        CHECK(p->get_stdout() != 0);
        CHECK(p->get_stderr() == 0);
        
        std::string result = read_all(* p->get_stdout() );        
        p->wait();
        CHECK( result.size() > 0);
        CHECK_EQUAL(0, p->get_exit_code());
    }
    
    TEST(subprocess_read_write) {
        std::string result;
        
        {
            using tinfra::read_all;
            using tinfra::write_all;
        
            std::auto_ptr<subprocess> p = tinfra::subprocess::create();
            
            p->set_stdin_mode(subprocess::REDIRECT);
            p->set_stdout_mode(subprocess::REDIRECT);
                    
            p->start("sort");
            
            CHECK(p->get_stdin()  != 0);
            CHECK(p->get_stdout() != 0);
            CHECK(p->get_stderr() == 0);
            
            write_all(* p->get_stdin(), "zz\r\ncc\r\nbb\r\naa\r\n");
            p->get_stdin()->close();

            result = read_all(* p->get_stdout() );
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
            std::string erased_variable = "OS";
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

