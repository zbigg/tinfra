#include <tinfra/io/stream.h>
#include <tinfra/server.h>
#include <tinfra/cmd.h>
#include <tinfra/fmt.h>
#include <tinfra/string.h>

#include <stdlib.h>
#include <iostream>

using std::string;
string server_auth_token;
string editor_command = "D:\\progs\\jedit\\jedit\\jedit.cmd";

string get_editor_command(string const& filename)
{
    return tinfra::fmt("%s \"%s\"") % editor_command % filename;
}

int getc(tinfra::io::stream* s)
{
    char b;
    if( s->read(&b, 1) == 1 ) {        
        return b;
    } else {
        return -1;
    }
}
void getline(tinfra::io::stream* in, std::string& s)
{
    while( true ) {
        const int c = getc(in);
        if( c == -1 ) {
            return;
        }        
        s.append(1, (char)c);
        if( c == '\n' ) {
            return;
        }
    }
}

class editor_server: public tinfra::net::Server {
public:
    virtual void onAccept(std::auto_ptr<tinfra::io::stream> client, std::string const& pa) {        
        // read line1 -> auth
        // read line2 -> path
        std::cerr << "got conn: " << pa << "\n";
        string client_auth_token;
        string path;
        try { 
            getline(client.get(), client_auth_token);
            getline(client.get(), path);
            tinfra::strip_inplace(client_auth_token);
            tinfra::strip_inplace(path);
            
            std::cerr << "got auth: " << client_auth_token << "\n";
            std::cerr << "got path: " << path << "\n";
            
            if( client_auth_token != server_auth_token ) {
                std::cerr << "error: bad auth\n";
                return;
            }
            std::string command = get_editor_command(path);
            std::cerr << "ok: executing " << command << "\n";
            system(command.c_str());
        } catch( std::exception const& e) {
            std::cerr << "error during execution: " << e.what() << "\n";
        }
    }
};



int editor_server_main(int, char**)
{
    using std::cout;
    
    // TODO: add token randomization    
    server_auth_token = "12345";
    cout << "auth_token=" << server_auth_token << "\n";
    
    editor_server server;
    server.bind("", 10666);
    
    server.run();
    return 0;
}

TINFRA_MAIN(editor_server_main)
