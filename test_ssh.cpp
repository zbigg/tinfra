
#include <unittest++/UnitTest++.h>


//#include "tinfra/ssh.h"
#include "ssh.h"

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
}

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++

