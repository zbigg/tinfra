#include "tinfra/io/stream.h"
#include "tinfra/io/socket.h"
#include "tinfra/fmt.h"
#include "tinfra/os_common.h"

#include "tinfra/string.h" // debug only

#include "tinfra/win32.h"
#include <iostream> // debug only

#include <stdexcept>

#ifdef _WIN32
#include <winsock.h>

#define TS_WINSOCK

#else

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#define TS_BSD
#endif

using namespace std;

namespace tinfra {
namespace io {
namespace socket {

/*
class stream {
    virtual ~stream() {}
    enum {
        start,
        end,
        current
    } seek_origin;        
    void close() = 0;
    int seek(int pos, seek_origin origin = start) = 0;
    int read(char* dest, int size) = 0;
    int write(const char* data, int size) = 0;
};
*/

#ifdef TS_WINSOCK
typedef SOCKET socket_type;
#else
typedef int    socket_type;
#endif

static const socket_type invalid_socket = static_cast<socket_type>(-1);

static int  close_socket_nothrow(socket_type socket);
static void close_socket(socket_type socket);
static void throw_socket_error(const char* message);

// TODO this log facility should be tinfra-wide
#ifdef _DEBUG
static void L(const std::string& msg)
{
    //std::cerr << "socket: " << escape_c(msg) << std::endl;
}
#else
#define L(a) (void)0
#endif

class socketstream: public stream {
    socket_type socket_;
public:
    socketstream(socket_type socket): socket_(socket) {}
    ~socketstream() {
        if( socket_ != invalid_socket ) {            
            if( close_socket_nothrow(socket_) == -1 ) {
                // TODO: add silent failures reporting
                // int err = socket_get_last_error();
                // tinfra::silent_failure(fmt("socket close failed: %i" % blabla )
            }
        }
    }
    void close() {        
        socket_type tmp(socket_);
        socket_ = invalid_socket;
        close_socket(tmp);
    }
    
    int seek(int pos, stream::seek_origin origin)
    {
        throw io_exception("sockets don't support seek()");
    }
    
    int read(char* data, int size)
    {
        L(fmt("%i: reading ...") % socket_);
        int result = ::recv(socket_, data ,size, 0);
        L(fmt("%i: readed %i '%s'") % socket_ % result % std::string(data,result));
        if( result == -1 ) {
            throw_socket_error("unable to read from socket");
        }
        return result;
    }
    
    int write(const char* data, int size)
    {
        L(fmt("%i: send '%s'") % socket_ % std::string(data,size));
        int result = ::send(socket_, data, size, 0);
        L(fmt("%i: sent %i") % socket_ % result);
        if( result == -1 ) {
            throw_socket_error("unable to write to socket");
        }
        return result;
    }
    
    void sync() 
    {
        // TODO: are sockets synchronized by default ? check it for unix/winsock
    }
    
    intptr_t native() const 
    {
        return socket_;
    }
    void release() 
    {
        socket_ = invalid_socket;
    }
    
    socket_type get_socket() const { return socket_; }
};

static void throw_socket_error(const char* message);
static void throw_socket_error(int error_code, const char* message);

static void ensure_socket_initialized()
{
#ifdef TS_WINSOCK
    static bool winsock_initialized = false;
    if( winsock_initialized ) return;
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    wVersionRequested = MAKEWORD(1, 1);

    err = WSAStartup(wVersionRequested, &wsaData);

    if (err != 0)
	throw_socket_error(err, "unable to initialize WinSock subsystem");

    if ( LOBYTE( wsaData.wVersion ) != 1 ||
	   HIBYTE( wsaData.wVersion ) != 1 ) {
        WSACleanup();
        errno = ENODEV;
        throw_socket_error(err, "unsupported WinSock version");
    }
    winsock_initialized = true;
#endif
}

static int  socket_get_last_error()
{
#ifdef TS_WINSOCK
    return WSAGetLastError();
#else
    return errno;
#endif
}

static void throw_socket_error(int error_code, const char* message)
{
#ifdef TS_WINSOCK
    tinfra::win32::throw_system_error(error_code, message);
#else
    throw_errno_error(errno, message);
#endif
}

static void throw_socket_error(const char* message)
{
    throw_socket_error(socket_get_last_error(), message);
}

static int close_socket_nothrow(socket_type socket)
{
#ifdef TS_WINSOCK
    return ::closesocket(socket);
#else
    return ::close(socket);
#endif    
}

static void close_socket(socket_type socket)
{
    int rc = close_socket_nothrow(socket);
    if( rc == -1 ) 
        throw_socket_error("unable to close socket");
}

static socket_type create_socket()
{
    ensure_socket_initialized();
    socket_type result = ::socket(AF_INET,SOCK_STREAM,0);
    if( result == invalid_socket ) 
        throw_socket_error("socket creation failed");
    return result;
}
#ifndef INADDR_NONE
#define INADDR_NONE -1
#endif

static void get_inet_address(const char* address, int rport, struct sockaddr_in* sa)
{
    ensure_socket_initialized();
    if( address == 0 ) throw std::invalid_argument("null address pointer");
    
    std::memset(sa,0,sizeof(*sa));
    sa->sin_family = AF_INET;
    sa->sin_port = htons((short)rport);

    ::in_addr     ia;
    unsigned long ian =  ::inet_addr(address);
    ia.s_addr = ian;
    if( ian == INADDR_NONE ) {
        ::hostent*    ha;
        ha = ::gethostbyname(address);
        if( ha == NULL ) {
#ifdef TS_WINSOCK
            throw_socket_error(fmt("unable to resolve '%s'") % address);
#else
            std::string message = fmt("unable to resolve '%s': %s") % address % hstrerror(h_errno);
            switch( h_errno ) {
            case HOST_NOT_FOUND:
            case NO_ADDRESS:
            // TODO: check on uix machine: NO_DATA should be also domain error
            // case NO_DATA: 
                throw std::domain_error(message);
            default:
                throw std::runtime_error(message);
            }
#endif
        }            
    	std::memcpy(&sa->sin_addr, ha->h_addr, ha->h_length);
    } else {
        /* found with inet_addr or inet_aton */
        std::memcpy(&sa->sin_addr,&ia,sizeof(ia));
    }
    sa->sin_port = htons((short)rport);
}

stream* open_client_socket(char const* address, int port)
{    
    ::sockaddr_in sock_addr;
    
    get_inet_address(address, port, &sock_addr);
    
    socket_type s = create_socket();
    
    if( ::connect(s, (struct sockaddr*)&sock_addr,sizeof(sock_addr)) != 0 ) {
        int error_code = socket_get_last_error();
        close_socket_nothrow(s);
        throw_socket_error(error_code, fmt("unable to connect to '%s:%i'") % address % port);
    }
    return new socketstream(s);
}

stream* open_server_socket(char const* listening_host, int port)
{
    ::sockaddr_in sock_addr;
    std::memset(&sock_addr,0,sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    if( listening_host ) {
        get_inet_address(listening_host, port, &sock_addr);
    } else {
        sock_addr.sin_port = htons((short) port);
    }
    
    socket_type s = create_socket();
    
    {
        int r = 1;
        if( ::setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)(void*)&r, sizeof(r)) ) {
            // TODO: it should be warning
            std::cerr << "unable to set SO_REUSEADDR=1 on socket" << std::endl;
        }
    }
    if( ::bind(s,(struct sockaddr*)&sock_addr, sizeof(sock_addr)) != 0 ) {
        int error_code = socket_get_last_error();
        close_socket_nothrow(s);
        throw_socket_error(error_code, "bind failed");
    }

    if( ::listen(s,5) != 0 ) {
        int error_code = socket_get_last_error();
        close_socket_nothrow(s);
        throw_socket_error(error_code, "listen failed");
    }
    
    return new socketstream(s);
}

stream* accept_client_connection(stream* server_socket)
{
    socketstream* socket = dynamic_cast<socketstream*>(server_socket);
    if( !socket ) 
        throw std::invalid_argument("accept: not a socketstream");
    
    socket_type accept_sock = ::accept(socket->get_socket(), NULL, NULL );
    if( accept_sock == invalid_socket ) 
        throw_socket_error("accept failed");
        
    return new socketstream(accept_sock);
}

} } } //end namespace tinfra::io::socket

