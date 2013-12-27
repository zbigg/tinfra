//
// Copyright (c) 2010, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_socket_h_included
#define tinfra_socket_h_included

#include "platform.h" // for intptr_t
#include "stream.h"
#include "tstring.h"

#include <memory>

#define TINFRA_INVARIANT(a)

namespace tinfra {

class socket {    
public:
    typedef intptr_t handle_type;

    /// Create socket object from already opened socket.
    ///
    /// Ownership of handle h is taken by this socket instance.
    socket(handle_type h);

    /// Destroy socket object and underlying handle.
    ///
    /// Closes socket (if not already closed), ignores any that might
    /// occur while closing/shutdown operation.
    virtual ~socket();

    /// Close socket.
    ///
    /// Might throw exception in case of incorrect socket shutdown or
    /// inability to deliver some parts of already written data.
    void     close();

    /// Retrieve socket object handle.
    handle_type handle() const { return this->handle_; }
    
    void set_blocking(bool blocking);

protected:
    
    handle_type handle_;
private:
    // noncopyable
    socket(socket const&);
    socket& operator=(socket const& other);
};

class client_stream_socket: public socket, 
                            public input_stream, 
                            public output_stream
{
public:
    client_stream_socket(handle_type h);
    
    // input_stream interface
    int read(char* dest, int size);

    // output_stream interface
    int write(const char* data, int size);
    void sync();
    
    // shared interface
    void close();
};

//
// implementation details
//

namespace detail {
int  close_socket_nothrow(socket::handle_type socket);
void throw_socket_error(const char* message);
void throw_socket_error(int error_code, const char* message);
bool last_socket_error_is_interruption();
}

} // end namespace tinfra

#endif // tinfra_socket_h_included
