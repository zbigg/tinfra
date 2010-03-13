#include "ssh.h"

#include <tinfra/subprocess.h>
#include <tinfra/runtime.h>
#include <tinfra/tinfra_lex.h>
#include <tinfra/fmt.h>
#include <tinfra/path.h>
#include <tinfra/os_common.h>
#include <tinfra/trace.h>
#include <iostream>

#ifndef _WIN32
#include <cerrno>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#endif

TINFRA_MODULE_TRACER(tinfra_ssh);


namespace tinfra {
namespace ssh {

TINFRA_USE_TRACER(tinfra_ssh);

std::ostream& operator <<(std::ostream& s, std::vector<std::string> const& a) {
    for(int i = 0; i < a.size(); ++i ) {
        if( i > 0 )
                s << " ";
        s << a[i];
    }
    return s;
}

using std::auto_ptr;
using tinfra::subprocess;

#ifndef WIN32
static std::string make_safe_script(std::string const& contents )
{
    std::string name = tinfra::path::tmppath();
    int fd = ::open(name.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0700);
    if( fd < 0) 
        tinfra::throw_errno_error(errno, "open safe file failed");
    
    size_t w = write(fd, contents.data(), contents.size());
    if( w != contents.size() ) {
        ::close(fd);
        tinfra::throw_errno_error(errno, "write safe file failed");
    }
    
    if( ::close(fd) < 0 ) 
        tinfra::throw_errno_error(errno, "open safe file failed");
    return name;
}

#else
static std::string make_safe_script(std::string const& contents)
{
    assert(false);
}
#endif
static std::string get_executable(std::string const& rule, std::string const& def)
{
    std::string result;
    if( rule.empty() ) 
        result = def;
    else
        result = rule;
    
    std::string abs_path = tinfra::path::search_executable(result);
    if( abs_path.empty() ) 
        throw std::runtime_error(fmt("unable to find '%s' in PATH") % result);
    
    return abs_path;
}

void start_openssh(subprocess* sp, connection_settings const& settings, command_line const& user_command)
{
    command_line cmd;    
    std::string program = get_executable(settings.provider, "ssh");
    
    cmd.push_back(program);
    
    if( settings.forward_agent ) 
        cmd.push_back("-A");
    else
        cmd.push_back("-a");
    
    if( true ) // TODO: compression flag
        cmd.push_back("-C");
    
    if( settings.subsystem_invocation )
        cmd.push_back("-s");
    
    if( settings.server_ssh_port != 22 ) {
        cmd.push_back("-p");
        std::string sport;
        to_string(settings.server_ssh_port, sport);
        cmd.push_back(sport);
    }
    
    if( !settings.priv_key_filename.empty() ) {
        cmd.push_back("-i");
        cmd.push_back(settings.priv_key_filename);
    }
    
    if( !settings.login_name.empty() ) {
        cmd.push_back("-l"); // - l like love
        cmd.push_back(settings.login_name);
    }
    
    if( settings.protocol == 1 ) {
        cmd.push_back("-1"); // - one
    }
    
    tinfra::environment_t env = tinfra::get_environment();
    bool use_env = false;
    if( settings.use_agent == false ) {
        use_env = true;
        env.erase(std::string("SSH_AUTH_SOCK"));
    }
    if( settings.password.size() > 0  ) {
        use_env = true;
        if( env["DISPLAY"] == "") 
            env["DISPLAY"] = "dummy";
        std::string tmppath = tinfra::path::tmppath();
        
        std::string ssh_ask_pass_path = make_safe_script(
            fmt(
                "#!/bin/sh\n"
                "echo -n '%s'\n" )  % settings.password);
        env["SSH_ASKPASS"] = ssh_ask_pass_path;
        std::cerr << "warning, remove this file: " << ssh_ask_pass_path << "\n";
    }
    cmd.push_back(settings.server_address);
    cmd.insert( cmd.end(), user_command.begin(), user_command.end() );
    if( use_env )
        sp->set_environment(env);
    
    TINFRA_TRACE_VAR(cmd);
    sp->start(cmd);
}

void start_putty(subprocess* sp, connection_settings const& settings, command_line const& user_command)
{
    command_line cmd;    
    std::string program = get_executable(settings.provider, "plink");
    
    cmd.push_back(program);
    
    cmd.push_back("-batch");
    
    if( settings.use_agent ) 
        cmd.push_back("-agent");
    else
        cmd.push_back("-noagent");
    
    if( settings.forward_agent ) 
        cmd.push_back("-A");
    else
        cmd.push_back("-a");
    
    if( true ) // TODO: compression flag
        cmd.push_back("-C");
    
    if( settings.subsystem_invocation )
        cmd.push_back("-s");
    
    if( settings.server_ssh_port != 22 ) {
        cmd.push_back("-P");
        std::string sport;
        to_string(settings.server_ssh_port, sport);
        cmd.push_back(sport);
    }
    
    if( !settings.priv_key_filename.empty() ) {
        cmd.push_back("-i");
        cmd.push_back(settings.priv_key_filename);
    }
    
    if( !settings.login_name.empty() ) {
        cmd.push_back("-l"); // - l like love
        cmd.push_back(settings.login_name);
    }
    
    if( settings.protocol == 1 ) 
        cmd.push_back("-1"); // - one
    
    if( settings.password.size() > 0 ) {
        cmd.push_back("-pw");
        cmd.push_back(settings.password);
    }
    
    cmd.push_back(settings.server_address);
    cmd.insert( cmd.end(), user_command.begin(), user_command.end() );
    
    TINFRA_TRACE_VAR(cmd);
    sp->start(cmd);
}

class subprocess_ssh_connection: public ssh::connection {
    auto_ptr<subprocess> sp;
public:
    subprocess_ssh_connection(auto_ptr<subprocess> sp_):
        sp(sp_)
    {}
    
    stream* get_output() {
        return sp->get_stdin();
    }

    stream* get_input() {
        return sp->get_stdout();
    }
};

class external_subprocess_factory: public connection_factory {
public:
    auto_ptr<ssh::connection> open_connection(
        connection_settings const& settings, 
        command_line const& command)
    {
        auto_ptr<subprocess> sp = tinfra::subprocess::create();
        
        sp->set_stdin_mode(subprocess::REDIRECT);
        sp->set_stdout_mode(subprocess::REDIRECT);
        // TODO: invent API&rules for stderr
        
        const size_t NOTFOUND = std::string::npos;
        
        // arghhh, refactor it somehow!
        bool use_ssh = false;
        bool use_putty = false;
        
        if( settings.provider == "" ) {
#ifdef _WIN32
            use_putty = tinfra::path::search_executable("plink") != "";
#endif
            if( !use_putty )
                use_ssh = tinfra::path::search_executable("ssh") != ""; 

        }
        
        if( settings.provider.find("plink") != NOTFOUND ) {
            use_putty = true;
        } else if( settings.provider.find("ssh") != NOTFOUND ) {
            use_ssh = true;
        }
        
        if( use_putty  ) {
            start_putty(sp.get(), settings, command);
        } else if( use_ssh ){
            start_openssh(sp.get(), settings, command);
        } else {
            throw std::logic_error(fmt("unknown ssh provider '%s')") % settings.provider );
        }   
        return auto_ptr<connection>(new subprocess_ssh_connection(sp));
    }
};


connection::~connection()
{
}

external_subprocess_factory default_factory;

connection_factory& connection_factory::get()
{
    return default_factory;
}

connection_factory::~connection_factory()
{
}

} } // end namespace tinfra::ssh

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++

