//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/cmd.h"
#include "tinfra/runtime.h"
#include "tinfra/exeinfo.h"
#include "tinfra/trace.h"
#include "tinfra/stream.h"
#include "tinfra/logger.h"
#include "tinfra/fs.h"

#include <string>
#include <exception>

#include <cstdio>
#include <cstdlib>

namespace tinfra {
namespace cmd {

app::app()
    : program_name_("<unknown>")
    , error_count_(0)
    , warning_count_(0)
{
    
}

app::~app()
{
}

void app::program_name(std::string const& p)
{
    const std::size_t pi = p.find_last_of("/\\");
    if( pi != std::string::npos)
        program_name_ = p.substr(pi+1);
    else
        program_name_ = p;
}

std::string const& app::program_name() const
{
    return program_name_;
}

unsigned app::error_count() const
{
    return error_count_;
}

void app::fail(tstring const& msg)
{
    tinfra::log_fail(msg);
    error_count_ += 1;
    exit(1);
}

void app::warning(tstring const& msg)
{
    tinfra::log_warning(msg);
    warning_count_ += 1;
}

void app::silent_exception(tstring const& msg)
{
    tinfra::log_warning(std::string("ignored exception: ") + msg.str());
    warning_count_ += 1;
}

void app::error(tstring const& msg)
{
    tinfra::log_error(msg);
    error_count_ += 1;
}

void app::inform(tstring const& msg)
{
    tinfra::log_info(msg);
}

static app default_app;

app& app::get()
{
    return default_app; 
}

// the "main" that enables some runtime things
// called by TINFRA_MAIN.
// forwards control to real application main

int main_wrapper(int argc, char* argv[],int (*real_main)(int,char*[]))
{
    std::string real_name(argv[0]);
    try {
        real_name = tinfra::fs::realpath(real_name);
    } catch( std::runtime_error& e ) {
    }
    set_exepath(real_name);
        
    app::get().program_name(argv[0]);
    
    initialize_fatal_exception_handler();    
    
    try {
        tinfra::public_tracer::process_params(argc, argv);
        
        return real_main(argc, argv);
    } catch( std::exception& e) {
        app::get().fail(e.what());
    } catch( char* msg ) {
        app::get().fail(msg);
    } catch( std::string& msg ) {
        app::get().fail(msg);
    } catch( ... ) {
        app::get().fail("unknown exception");
    }
    return 1;
}

} } // end of namespace tinfra::cmd

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

