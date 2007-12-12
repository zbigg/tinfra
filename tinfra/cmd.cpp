#include "tinfra/cmd.h"
#include "tinfra/exception.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include <exception>

using namespace std;
namespace tinfra {
namespace cmd {

static void print_maybe_multiline(string const& PS1, string const& PS2, string const& message, ostream& out)
{
    // TODO: implement "multiline behaviour"
    unsigned start = 0;
    bool finished = false;
    do {
        if( start == message.size() ) break;
            
        unsigned eol = message.find_first_of('\n', start);
        unsigned len;
        if( eol == string::npos ) {
            if( start != message.size()-1 ) {
                len = string::npos;
                finished = true;
            } else {
                return;
            }
        } else {    
            unsigned pi = eol;
            if( pi > start+1 && message[eol-1] == '\r' ) --pi;
            len = pi-start;
        }
        if( len != 0 ) 
            cerr << (start == 0 ? PS1: PS2) << message.substr(start, len) << endl;            
        start = eol+1;
    } while( !finished );
}
static void print_maybe_multiline(string const& prefix, string const& message, ostream& out)
{
    print_maybe_multiline(prefix,prefix,message,out);
}

app::app()
    : program_name_("<unknown>")
    , error_count_(0)
    , warning_count_(0)
{
    
}

app::~app()
{
}

void app::program_name(string const& p)
{
    unsigned pi = p.find_last_of("/\\");
    if( pi != string::npos)
        program_name_ = p.substr(pi+1);
    else
        program_name_ = p;
}

string const& app::program_name() const
{
    return program_name_;
}

unsigned app::error_count() const
{
    return error_count_;
}

void app::fail(std::string const& msg)
{
    print_maybe_multiline(program_name() + ": fatal error: ", msg,cerr);
    exit(1);
}

void app::warning(std::string const& msg)
{
    print_maybe_multiline(program_name() + ": warning: ", msg,cerr);
    warning_count_ += 1;
}

void app::error(std::string const& msg)
{
    print_maybe_multiline(program_name() + ": error: ", msg,cerr);
    error_count_ += 1;
}

void app::inform(std::string const& msg)
{
    print_maybe_multiline(program_name() + ": ", msg,cerr);
    error_count_ += 1;
}
static app default_app;

app& app::get()
{
    return default_app; 
}


int main(int argc, char* argv[],int (*real_main)(int,char*[]))
{
    app::get().program_name(argv[0]);
    
    initialize_fatal_exception_handler();
    
    try {
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
