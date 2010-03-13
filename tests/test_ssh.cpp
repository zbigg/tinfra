
#include <unittest++/UnitTest++.h>


#include "tinfra/ssh.h"
#include <iostream>

#include "tinfra/option.h"
#include "tinfra/cmd.h"

SUITE(tinfra_ssh) {

    
    TEST(api_compilation_test)
    {
        
        tinfra::ssh::connection_settings settings;
        settings.provider="putty";
        settings.server_address = "localhost";
        settings.server_ssh_port = 22;
        settings.login_name = "test";
        settings.priv_key_filename = "test_key_name";
        settings.protocol = 2;
        settings.forward_agent = false;
        settings.use_agent = false;
        settings.subsystem_invocation = true;
        
        tinfra::ssh::connection_factory& f = tinfra::ssh::connection_factory::get();
        
        tinfra::ssh::command_line cmd;
        cmd.push_back("uname");
        cmd.push_back("-a");
        if( false ) {
            
            std::auto_ptr<tinfra::ssh::connection> c = f.open_connection(settings, cmd);
            
            c->get_output();
            c->get_input();
        }
    }
    
    tinfra::option<std::string> opt_server_host("", "ssh-server", "SSH server hostname to perform ssh functional test");
    tinfra::option<int>         opt_server_port(22, "ssh-port", "SSH server port to perform ssh functional test");
    tinfra::option<std::string> opt_remote_user("", "ssh-user", "remote user name to perform ssh functional test");
    tinfra::option<std::string> opt_remote_password("", "ssh-password", "remote user password to perform ssh functional test");
    
    
    TEST(password_login)
    {
        if( !opt_server_host.accepted() ) {
            tinfra::cmd::inform("test skipped (need --ssh-server option specified)");
            return;
        }
        
        tinfra::ssh::connection_settings settings;
        //settings.provider = "ssh";
        settings.server_address = opt_server_host.value();
        settings.server_ssh_port = opt_server_port.value();
        settings.login_name = opt_remote_user.value();
        settings.password   = opt_remote_password.value();
        settings.use_agent = false;
        settings.subsystem_invocation = false;
        
        tinfra::ssh::command_line cmd;
        cmd.push_back("uname");
        cmd.push_back("-a");
        
        std::auto_ptr<tinfra::ssh::connection> c = tinfra::ssh::connection_factory::get()
            .open_connection(settings, cmd);
            
        tinfra::io::stream* in = c->get_input();
        char ch;
        while( in->read(&ch, 1) > 0 ) {
            std::cout << ch;
        }
    }
}

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++

