#ifndef tinfra_tcp_socket_h_included
#define tinfra_tcp_socket_h_included

#include "stream.h"
#include "tstring.h"

#include <memory>

namespace tinfra {

class tcp_socket {    
public:
    typedef intptr_t handle_type;

    /// Create socket object with newly allocated handle.
    tcp_socket();

    /// Create socket object from already opened socket.
    ///
    /// Ownership of handle h is taken by tcp_socket instance.
    tcp_socket(handle_type h);

    /// Destroy socket object and underlying handle.
    virtual ~tcp_socket();

    /// Retrieve socket object handle.
    handle_type handle() const { return this->handle_; }
    
    void set_blocking(bool blocking);

protected:
    
    handle_type handle_;
private:
    // noncopyable
    tcp_socket(tcp_socket const&);
    tcp_socket& operator=(tcp_socket const& other);
};

class tcp_client_socket: public tcp_socket, public input_stream, public output_stream
{
public:
    tcp_client_socket(handle_type h);
    tcp_client_socket(tstring const& address, int port);
    ~tcp_client_socket();
    
    // input_stream interface
    int read(char* dest, int size);

    // output_stream interface
    int write(const char* data, int size);
    void sync();
    
    // shared interface
    void close();
};

class tcp_server_socket: public tcp_socket {
public:
    tcp_server_socket(tstring const& address, int port);
    ~tcp_server_socket();

    /// Accepts new connections.
    /// 
    /// Accepts new connection and create client socket object
    /// By default this operation is blocking, with non-blocking
    /// socket, this operation may return non-initialized pointer.
    std::auto_ptr<tcp_client_socket> accept(std::string& address);
};

} // end namespace tinfra

#endif // tinfra_tcp_socket_h_included
