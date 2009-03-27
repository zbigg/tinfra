
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
    
    TEST(password_login)
    {
        //std::string site = "localhost";
        //std::string login_name = "";
        //std::string password = "";
        
        tinfra::ssh::connection_settings settings;
        settings.provider = "plink";
        settings.server_address = site;
        settings.login_name = login_name;
        settings.password   = password;        
        settings.use_agent = false;
        settings.subsystem_invocation = false;
        
        tinfra::ssh::command_line cmd;
        cmd.push_back("uname");
        cmd.push_back("-a");
        
        std::auto_ptr<tinfra::ssh::connection> c = tinfra::ssh::connection_factory::get()
            .open_connection(settings, cmd);
            
        c->get_output();
        tinfra::io::stream* in = c->get_input();
        char ch;
        while( in->read(&ch, 1) > 0 ) {
        }
    }
}

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++

