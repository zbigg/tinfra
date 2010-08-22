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

using detail::throw_socket_error;
    
static void ensure_socket_subsystem_initialized()
{
#ifdef TS_WINSOCK
    static bool winsock_initialized = false;
    if( winsock_initialized ) 
        return;
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
    if( address.empty() ) 
        throw std::invalid_argument("null address pointer");
    
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
            const int e = h_errno;
            const std::string message = fmt("unable to resolve '%s': %s (%s)") % address % hstrerror(e) %  e;
            throw std::runtime_error(message);
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


static int create_tcp_socket()
{
    ensure_socket_subsystem_initialized();
    socket::handle_type result = ::socket(AF_INET,SOCK_STREAM,0);
    if( result == -1 ) 
        detail::throw_socket_error("socket creation failed");
    return result;
}


//
// tcp_client_socket
//

tcp_client_socket::tcp_client_socket(socket::handle_type h):
    client_stream_socket(h)
{
}

tcp_client_socket::tcp_client_socket(tstring const& address, int port):
    client_stream_socket(create_tcp_socket())
{
    ::sockaddr_in sock_addr;
    get_inet_address(address, port, &sock_addr);
    
    while( true ) {
        int rc = ::connect(handle(), (struct sockaddr*)&sock_addr,sizeof(sock_addr));
        if( rc != 0 && detail::last_socket_error_is_interruption() ) {
            TINFRA_TRACE_MSG("connect() call interrupted (EINTR), retrying");
            continue;
        }
        if( rc != 0 ) {
            throw_socket_error(fmt("connection to '%s:%i' failed") % address % port);
        }
        break;
    }
}

tcp_client_socket::~tcp_client_socket()
{
}
    

//
// tcp_server_socket
//
tcp_server_socket::tcp_server_socket(tstring const& address, int port):
    socket(create_tcp_socket())
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
        throw_socket_error(fmt("bind to '%s:%i' failed") % actual_address % port );
    }

    if( ::listen(handle(),5) != 0 ) {
        throw_socket_error("listen failed");
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
    
    socket::handle_type accept_sock;
    while( true ) {
        accept_sock = ::accept(handle(), (struct sockaddr*)&client_address, &addr_size );
        
        if( accept_sock == -1 && detail::last_socket_error_is_interruption()) {
            TINFRA_TRACE_MSG("accept() call interrupted (EINTR), retrying");
            continue;
        }
        
        if( accept_sock == -1 ) {
            throw_socket_error("accept failed");
        }
        break;
    }
    
    address = get_peer_address_string(client_address);
    return std::auto_ptr<tcp_client_socket>(new tcp_client_socket(accept_sock));
}

} //end namespace tinfra
