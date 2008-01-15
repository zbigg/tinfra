#ifndef __tinfra__io__socket_h__
#define __tinfra__io__socket_h__

namespace tinfra {
namespace io {
    
class stream;

namespace socket {
    
stream* open_client_socket(char const* address, int port);
stream* open_server_socket(char const* listening_host, int port);
stream* accept_client_connection(stream* server_socket);

class Server {
    std::auto_ptr<stream> server_socket_;
    bool stopped_;
public:
    Server();
    Server(const char* address, int port);

    void bind(const char* address, int port);
    void run();
    void stop();
    bool stopped() const { return stopped_; }
protected:

    virtual void onAccept(std::auto_ptr<stream> client) = 0;
};

} } }

#endif
