//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/platform.h"

#include "tinfra/subprocess.h"
#include "tinfra/fmt.h"

#include <vector>
#include <string>
#include <stdexcept>
namespace tinfra {
    
static void read_file(tinfra::io::stream* s, std::string& data)
{
    char buf[1024];
    int r;
    while( ( r = s->read(buf, sizeof(buf))) > 0 ) {
        data.append(buf, r);
    }
}

static std::string capture_command(std::string const& command, environment_t const* env);

std::string capture_command(std::string const& command)
{
    return capture_command(command, 0);
}

std::string capture_command(std::string const& command, environment_t const& env)
{
    return capture_command(command, &env);
}

std::string capture_command(std::string const& command, environment_t const* env)
{
    std::auto_ptr<subprocess> p = tinfra::subprocess::create();
                
    p->set_stdout_mode(subprocess::REDIRECT);
    if( env )
        p->set_environment(*env);

    std::string result;
    p->start(command.c_str());
    
    read_file(p->get_stdout(), result);
    
    p->wait();
    const int exit_code = p->get_exit_code();
    
    if( exit_code != 0 ) {
        throw std::runtime_error(fmt("command '%s' failed status %s") % command % exit_code);
    }
    
    return result;
}

void start_detached(tstring const& command, environment_t const* env);

void start_detached(tstring const& command)
{
    start_detached(command, 0);
}

void start_detached(tstring const& command, environment_t const& env)
{
    start_detached(command, &env);
}

void start_detached(tstring const& command, environment_t const* env)
{
    std::auto_ptr<subprocess> p = subprocess::create();
    p->set_stdout_mode(subprocess::NONE);
    p->set_stdin_mode(subprocess::NONE);
    p->set_stderr_mode(subprocess::NONE);
    
    if( env )
        p->set_environment(*env);
    p->start(command.str().c_str());
    p->detach();
}
//
// tinfra global stub for posix
// @deprecated

subprocess* create_subprocess() {
    return subprocess::create().release();
}

} // end of namespace tinfra

