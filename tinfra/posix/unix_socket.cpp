//
// Copyright (c) 2010, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/unix_socket.h"

#include "tinfra/fmt.h"

#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <stdexcept> // for std::runtime_error

namespace tinfra {

// code based on
// unix socket server/accepted/client socket creator
// http://www.cs.cf.ac.uk/Dave/C/node28.html#SECTION002810000000000000000
    
static int create_unix_socket()
{
    const int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if( fd == -1 ) {
        throw_errno_error(errno, "socket(AF_UNIX) failed");
    }
    return fd;
}

//
// unix_server_socket implementation
//

unix_server_socket::unix_server_socket(handle_type h):
    socket(h)
{
}

unix_server_socket::unix_server_socket(tstring const& path, 
                                       unix_server_socket::creation_policy policy)
:
    socket(create_unix_socket())
{
    struct sockaddr_un address;

    address.sun_family = AF_UNIX;
    // TODO: assert sun_path on length
    // TODO: convert from utf-8 to FS charset
    strncpy(address.sun_path, path.data(), path.size());
    address.sun_path[path.size()] = 0;
    
    if( tinfra::fs::exists(path)) {
        if( policy == FAIL_IF_EXISTS ) {
            throw std::runtime_error(fmt("server socket path ('%s') already exists") % path);
        }
        // TODO: handle error
        ::unlink(name);
    }
    
    int address_len = sizeof(address.sun_family) + strlen(address.sun_path);
    
    if (::bind(fd, reinterpret_cast<sockaddr*>(&address), address_len) < 0) {
        throw_errno_error(errno, "bind failed");
    }
    
    if (::listen(handle(), 5) < 0) {
        throw_errno_error(errno, "listen failed");
    }
}

unix_server_socket::~unix_server_socket()
{
}

std::auto_ptr<unix_client_socket> unix_server_socket::accept();
{
    struct sockaddr_un client_address;
    socklen_t address_len;
    
    // TODO: handle EINTR
    int client_fd = accept(handle(), reinterpret_cast<sockaddr*>(&client_address), &address_len);
    if (client_fd < 0) {
        throw_errno_error(errno, "accept(AF_UNIX) failed");
    }
    
    return std::auto_ptr( new unix_client_socket(client_fd) );
}

//
// unix_client_socket
//

unix_client_socket::unix_client_socket(socket::handle_type h):
    client_stream_socket(h)
{
}

unix_client_socket::unix_client_socket(tstring const& path):
    client_stream_socket(create_unix_socket())
{
    struct sockaddr_un address;
    // TODO: assert sun_path on length
    // TODO: convert from utf-8 to FS charset
    strncpy(address.sun_path, path.data(), path.size());
    address.sun_path[path.size()] = 0;
 
    int address_len = sizeof(address.sun_family) + strlen(address.sun_path);
    
    // TODO: handle EINTR
    if( connect(handle(), reinterpret_cast<sockaddr*>(&address), address_len) < 0 ) {
        throw_errno_error(errno, fmt("connect to unix socket ('%s') failed") % path);
    }
    
}

} // end namespace tinfra

