#include "ssh.h"

#include <tinfra/subprocess.h>
#include <tinfra/tinfra_lex.h>
#include <tinfra/fmt.h>

namespace tinfra {
namespace ssh {

using std::auto_ptr;
using tinfra::subprocess;

static std::string get_executable(std::string const& rule, std::string const& def)
{
    if( rule.empty() )
        return def;
    return rule;
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
    
    cmd.insert( cmd.end(), user_command.begin(), user_command.end() );
    sp->start(cmd);
}

void start_putty(subprocess* sp, connection_settings const& settings, command_line const& user_command)
{
    command_line cmd;    
    std::string program = get_executable(settings.provider, "plink");
    
    cmd.push_back(program);
    
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
    
    cmd.insert( cmd.end(), user_command.begin(), user_command.end() );
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
        
        if( settings.provider.find("ssh") != NOTFOUND) {
            start_openssh(sp.get(), settings, command);
        } else if( settings.provider.find("plink") != NOTFOUND ) {
            start_putty(sp.get(), settings, command);
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

} } // end namespace tinfra::ssh

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++
