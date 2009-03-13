#ifndef tinfra_ssh_h_
#define tinfra_ssh_h_

#include <tinfra/io/stream.h>

#include <string>
#include <vector>
#include <memory>

namespace tinfra {
namespace ssh {

struct connection_settings {
    std::string provider;
    
    std::string server_address;
    int         server_ssh_port;
    
    std::string login_name;
    std::string password;
    std::string priv_key_filename;
    
    int         protocol;
    
    bool        forward_agent;
    bool        use_agent;
    
    bool        subsystem_invocation;
    
    std::vector<std::string> provider_options;
    
    /// costructor to initialize POD types
    /// to default values
    connection_settings():
        server_ssh_port(22),
        protocol(2),
        forward_agent(true),
        use_agent(true),
        subsystem_invocation(false)
    {
    }
};

std::ostream& operator <<(std::ostream&, connection_settings const&);

typedef std::vector<std::string> command_line;

class connection {
public:
    typedef tinfra::io::stream stream;
    
    virtual ~connection();
    
    // get output stream (you write there)
    virtual stream* get_output() = 0;
    
    // get input stream (you read from there)
    virtual stream* get_input() = 0;
};

class connection_factory {
public:
    virtual std::auto_ptr<connection> open_connection(
        connection_settings const& cs, 
        command_line const& command) = 0;
    
    
    static connection_factory& get();

    virtual ~connection_factory();
};

} } // end namespace tinfra::ssh

#endif // tinfra_ssh_h_

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++

