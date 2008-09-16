//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_io_stream_h_
#define __tinfra_io_stream_h_

#include <ios>
#include <streambuf>

#include "tinfra/exception.h"

namespace tinfra { 
namespace io {
    
class io_exception: public generic_exception {
public:
    io_exception(std::string const& message): generic_exception(message) {}
};

class would_block: public std::runtime_error {
public:
    would_block(std::string const& m): std::runtime_error(m) {}

};

typedef std::ios::openmode openmode;

class stream {
public:
    virtual ~stream() { }
    enum seek_origin {
        start,
        end,
        current
    };
    virtual void close() = 0;
    virtual int seek(int pos, seek_origin origin = start) = 0;
    virtual int read(char* dest, int size) = 0;
    virtual int write(const char* data, int size) = 0;
    virtual void sync() = 0;
    
    virtual intptr_t native() const = 0;
    virtual void release() = 0;
};

class dstream: public stream {
    stream* input_;
    stream* output_;        
public:
    dstream(stream* input, stream* output);
    
    virtual ~dstream();

    virtual void close();
    virtual int seek(int pos, seek_origin origin = start);
    virtual int read(char* dest, int size);
    virtual int write(const char* data, int size);

    virtual void sync();
    
    virtual intptr_t native() const;
    virtual void release();
};

stream* open_native(intptr_t handle);
stream* open_file(const char* name, openmode mode);

stream* open_command_pipe(char const* command, std::ios::openmode mode);
stream* open_anon_pipe();

stream* create_dstream(stream* input, stream* output);

//void    open_process(std::vector<std::string> args, process& result);
void copy(stream* in, stream* out, size_t max_bytes = 0);

class zstreambuf : public std::streambuf {
    //typedef std::streambuf::streamsize streamsize;
    
private:
    stream* stream_;
    bool    own_;
    char*   buffer_;
    int     buffer_size_;
    bool    own_buffer_;
    
public:
    zstreambuf(stream* stream = 0, bool own = false);
    zstreambuf(char const* name, std::ios::openmode mode = std::ios::in);

    /** Open a file in native filesystem */
    zstreambuf& open_file(char const* filename, std::ios::openmode mode = std::ios::in);

    /** Opens a client tcp connection. */
    zstreambuf& open_socket(char const* target, int port);

    /** Opens a pipe to command.

        Spawns a 'command' with stdout and/or stdin hijacked by this streambuf.
    */
    zstreambuf& open_pipe(char const* command, std::ios::openmode mode = std::ios::in);

    /** Opens an anonymous pipe

        That pipe can be used in-process to communicate between threads.
    */
    zstreambuf& open_pipe();

    void close();
    
    virtual ~zstreambuf();
    
    ///
    /// std::streambuf<char> contract implementation
    ///
    virtual void imbue (const std::locale &)
    {
    }
    virtual int_type overflow (int_type = -1);
    virtual int_type pbackfail (int_type = -1);
    virtual pos_type seekoff (off_type, std::ios::seekdir, std::ios::openmode=std::ios::in | std::ios::out);
    virtual pos_type seekpos (pos_type, std::ios::openmode=std::ios::in | std::ios::out);    
    virtual std::streambuf* setbuf (char *, std::streamsize);
    virtual std::streamsize showmanyc ();
    virtual int sync ();
    virtual int_type uflow ();
    virtual int_type underflow ();
    
    virtual std::streamsize xsgetn (char_type *__s, std::streamsize __n);
    virtual std::streamsize xsputn (const char_type *__s, std::streamsize __n);
    
    ///
    /// human readable Java like interface
    ///
    int read(char* data, int size);
    int write(char const* data, int size);
private:
    bool need_buf();
};

// TODO: it shouldn't be here
void copy(std::streambuf& in, std::streambuf& out);

} } // end namespace tinfra::io

#endif
