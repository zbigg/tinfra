//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "socket.h" // we implement this

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

namespace detail {

void throw_socket_error(int error_code, const char* message)
{
#ifdef TS_WINSOCK
    tinfra::win32::throw_system_error(error_code, message);
#else
    throw_errno_error(errno, message);
#endif
}

void throw_socket_error(const char* message)
{
#ifdef TS_WINSOCK
    int e = WSAGetLastError();
#else
    int e errno;
#endif
    throw_socket_error(e, message);
}

int close_socket_nothrow(socket::handle_type socket)
{
#ifdef TS_WINSOCK
    return ::closesocket(socket);
#else
    return ::close(socket);
#endif    
}

bool last_socket_error_is_interruption()
{
#ifdef TS_WINSOCK
    int e = WSAGetLastError();
    return e == WSAEINTR;
#else
    int e = errno;
    return e == EINTR;
#endif
}
} // end namespace tinfra::detail



//
// socket
//

socket::socket(socket::handle_type h):
    handle_(h)
{
}

socket::~socket()
{
    if( handle_ == -1 ) 
        return;
    while( true ) {
        int rc =  detail::close_socket_nothrow(handle());
        if( rc == -1 && detail::last_socket_error_is_interruption() ) {
            continue;
        }
        if( rc == -1 ) {
#ifdef TS_WINSOCK
            const int e = ::WSAGetLastError();
            const std::string e_string = tinfra::win32::get_error_string(e);
#else
            const int e = errno;
            const std::string e_string = tinfra::errno_to_string(e);
#endif
            TINFRA_LOG_ERROR(fmt("close() failed on socket: %s(%s), error ignored") % e_string % e);
        }
        break;
    }
}

void socket::close()
{
    TINFRA_INVARIANT( handle() != -1 );
    while( true ) {
        int rc = detail::close_socket_nothrow(handle());
        if( rc == -1 && detail::last_socket_error_is_interruption() ) {
            continue;
        }
        if( rc == -1 ) {
            detail::throw_socket_error("close() failed on socket");
        }
        break;
    }
    this->handle_ = -1;
}

void socket::set_blocking(bool blocking)
{
    
#ifdef TS_WINSOCK
    unsigned long block = blocking ? 0 : 1;
    if( ioctlsocket(handle(), FIONBIO, &block) < 0 )
        detail::throw_socket_error("set_blocking: ioctlsocket(FIONBIO) failed");
#else
    int flags = fcntl( handle(), F_GETFL );
    if( flags < 0 )
        detail::throw_socket_error("set_blocking: fcntl(F_GETFL) failed");
    if( !blocking )
        flags |= O_NONBLOCK;
    else
        flags &= ~(O_NONBLOCK);
    if( fcntl( handle(), F_SETFL, flags ) < 0 )
        detail::throw_socket_error("set_blocking: fcntl(F_SETFL) failed");
#endif

}

//
// tcp_client_socket
//

client_stream_socket::client_stream_socket(socket::handle_type h):
    socket(h)
{
}
    
    // input_stream interface
int 
client_stream_socket::read(char* dest, int size)
{
    TINFRA_INVARIANT( handle() != -1 );
    while( true ) {
        int result = ::recv(handle(), dest ,size, 0);
        if( result == -1 && detail::last_socket_error_is_interruption() ) {
            TINFRA_TRACE_MSG("recv() call interrupted (EINTR), retrying");
            continue;
        }
        if( result == -1 ) {
            detail::throw_socket_error("recv() failed when reading socket");
        }
        return result;
    }
}

    // output_stream interface
int 
client_stream_socket::write(const char* data, int size)
{
    TINFRA_INVARIANT( handle() != -1 );
    while( true ) {
        int result = ::send(handle(), data, size, 0);
        if( result == -1 && detail::last_socket_error_is_interruption() ) {
            TINFRA_TRACE_MSG("send() call interrupted (EINTR), retrying");
            continue;
        }
        if( result == -1 ) {
            detail::throw_socket_error("send() failed when reading socket");
        }
        return result;
    }
}

void 
client_stream_socket::sync()
{
    TINFRA_INVARIANT( handle() != -1 );
}
    
    // shared interface
void 
client_stream_socket::close()
{
    socket::close();
}

} //end namespace tinfra

