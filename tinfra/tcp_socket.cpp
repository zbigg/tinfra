//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tcp_socket.h" // we implement this

#include "tinfra/fmt.h"
#include "tinfra/os_common.h"

//#include "tinfra/string.h" // debug only

#include "tinfra/win32.h"
#include "tinfra/trace.h"
#include <stdexcept>

#ifdef _WIN32
#include <winsock.h>

#define TS_WINSOCK

typedef int socklen_t;

#else

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>

#define TS_BSD
#endif

namespace tinfra {

#ifdef TS_WINSOCK
typedef SOCKET socket_type;
#else
typedef int    socket_type;
#endif

static const socket_type invalid_socket = static_cast<socket_type>(-1);

// local helpers
static void ensure_socket_subsystem_initialized();
static int  close_socket_nothrow(socket_type socket);
static void close_socket(socket_type socket);
static void throw_socket_error(const char* message);
static void throw_socket_error(int error_code, const char* message);

static void ensure_socket_subsystem_initialized()
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
        err = WSAVERNOTSUPPORTED;
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

#ifndef INADDR_NONE
#define INADDR_NONE -1
#endif

#if !defined(TS_WINSOCK) && !defined(HAVE_HSTRERROR)
static const char* hstrerror(int error_code)
{
	return "host not found";
}
#endif

static void get_inet_address(tstring const& address, int rport, struct sockaddr_in* sa)
{
    string_pool local_pool;
    if( address.empty() ) throw std::invalid_argument("null address pointer");
    
    std::memset(sa,0,sizeof(*sa));
    sa->sin_family = AF_INET;
    sa->sin_port = htons((short)rport);

    ::in_addr     ia;
    unsigned long ian =  ::inet_addr(address.c_str(local_pool));
    ia.s_addr = ian;
    if( ian == INADDR_NONE ) {
        ::hostent*    ha;
        ha = ::gethostbyname(address.c_str(local_pool));
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

static std::string get_peer_address_string(sockaddr_in const& address)
{
    char buf[64] = "0.0.0.0";
#ifdef HAVE_INET_NTOP
    if( inet_ntop(AF_INET, &address.sin_addr, buf, sizeof(buf)) == 0 ) {
        return "<unknown>";
    }
#elif defined TS_WINSOCK
    strncpy( buf, inet_ntoa(address.sin_addr), sizeof(buf));    
#endif
    return tinfra::fmt("%s:%i") % buf % ntohs(address.sin_port);
}


//
// tcp_socket
//

tcp_socket::tcp_socket()
{
    ensure_socket_subsystem_initialized();
    socket_type result = ::socket(AF_INET,SOCK_STREAM,0);
    if( result == invalid_socket ) 
        throw_socket_error("socket creation failed");
    this->handle_ = result;
}

tcp_socket::tcp_socket(tcp_socket::handle_type h):
    handle_(h)
{
}

tcp_socket::~tcp_socket()
{
    if( handle_ != invalid_socket )
        close_socket_nothrow(handle());
}

void tcp_socket::set_blocking(bool blocking)
{
    
#ifdef TS_WINSOCK
    unsigned long block = blocking ? 0 : 1;
    if( ioctlsocket(handle(), FIONBIO, &block) < 0 )
        throw_socket_error("set_blocking: ioctlsocket(FIONBIO) failed");
#else
    int flags = fcntl( handle(), F_GETFL );
    if( flags < 0 )
        throw_socket_error("set_blocking: fcntl(F_GETFL) failed");
    if( !blocking )
        flags |= O_NONBLOCK;
    else
        flags &= ~(O_NONBLOCK);
    if( fcntl( handle(), F_SETFL, flags ) < 0 )
        throw_socket_error("set_blocking: fcntl(F_SETFL) failed");
#endif

}

//
// tcp_client_socket
//

tcp_client_socket::tcp_client_socket(tcp_socket::handle_type h):
    tcp_socket(h)
{
}

tcp_client_socket::tcp_client_socket(tstring const& address, int port)
{
    ::sockaddr_in sock_addr;
    get_inet_address(address, port, &sock_addr);
    
    if( ::connect(handle(), (struct sockaddr*)&sock_addr,sizeof(sock_addr)) != 0 ) {
        int error_code = socket_get_last_error();
	throw_socket_error(error_code, fmt("unable to connect to '%s:%i'") % address % port);
    }
}

tcp_client_socket::~tcp_client_socket()
{
}
    
    // input_stream interface
int 
tcp_client_socket::read(char* dest, int size)
{
    int result = ::recv(handle(), dest ,size, 0);
    if( result == -1 ) {
        throw_socket_error("unable to read from socket");
    }
    return result;
}

    // output_stream interface
int 
tcp_client_socket::write(const char* data, int size)
{
    int result = ::send(handle(), data, size, 0);
    if( result == -1 ) {
        throw_socket_error("unable to write to socket");
    }
    return result;
}

void 
tcp_client_socket::sync()
{
}
    
    // shared interface
void 
tcp_client_socket::close()
{
}

//
// tcp_server_socket
//
tcp_server_socket::tcp_server_socket(tstring const& address, int port):
    tcp_socket()
{
    ::sockaddr_in sock_addr;
    std::memset(&sock_addr,0,sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    tstring actual_address = address;
    if( !address.empty() ) {
        get_inet_address(address, port, &sock_addr);
    } else {
        actual_address = "0.0.0.0";
        sock_addr.sin_port = htons((short) port);
    }
    
    {
        int r = 1;
        if( ::setsockopt(handle(), SOL_SOCKET, SO_REUSEADDR, (char*)(void*)&r, sizeof(r)) ) {
            // TODO: it should be warning
            TINFRA_LOG_ERROR("unable to set SO_REUSEADDR=1 on socket");
        }
    }   
    if( ::bind(handle(),(struct sockaddr*)&sock_addr, sizeof(sock_addr)) != 0 ) {
        int error_code = socket_get_last_error();
        throw_socket_error(error_code, fmt("bind to '%s:%i' failed") % actual_address % port );
    }

    if( ::listen(handle(),5) != 0 ) {
        int error_code = socket_get_last_error();
        throw_socket_error(error_code, "listen failed");
    }
}

tcp_server_socket::~tcp_server_socket()
{
}

std::auto_ptr<tcp_client_socket> 
tcp_server_socket::accept(std::string& address)
{
    socklen_t addr_size = sizeof(sockaddr_in);
    sockaddr_in client_address;
    socket_type accept_sock = ::accept(handle(), (struct sockaddr*)&client_address, &addr_size );
    
    if( accept_sock == invalid_socket ) {
        int error_code = socket_get_last_error();
	//if( error_means_blocked(error_code) )
	//    return std::auto_ptr<tcp_client_socket>();
        throw_socket_error(error_code, "accept failed");
    }
    
    address = get_peer_address_string(client_address);
    return std::auto_ptr<tcp_client_socket>(new tcp_client_socket(accept_sock));
}

} //end namespace tinfra

