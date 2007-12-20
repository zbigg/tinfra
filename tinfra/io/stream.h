#ifndef __tinfra_io_stream_h_
#define __tinfra_io_stream_h_

#include <zcompat/zpio.h>

#include <ios>
#include <streambuf>

#include "tinfra/exception.h"

namespace tinfra { 
    
class io_exception: public generic_exception {
public:
    io_exception(std::string const& message): generic_exception(message) {}
};
class zstreambuf : public std::streambuf {
    //typedef std::streambuf::streamsize streamsize;
    
private:
    ZSTREAM     _stream;
    bool        _own;
public:
    zstreambuf(ZSTREAM stream = 0, bool own = false);
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
    virtual int_type overflow (int_type=traits_type::eof());
    virtual int_type pbackfail (int_type=traits_type::eof());
    virtual pos_type seekoff (off_type, std::ios::seekdir, std::ios::openmode=std::ios::in | std::ios::out);
    virtual pos_type seekpos (pos_type, std::ios::openmode=std::ios::in | std::ios::out);    
    //virtual basic_streambuf< char_type, _Traits >* setbuf (char_type *, streamsize)    
    virtual std::streamsize showmanyc ();
    virtual int sync ();
    virtual int_type uflow ();
    virtual int_type underflow ();
    
    virtual std::streamsize xsgetn (char_type *__s, std::streamsize __n)
    {
        return read(__s, __n);
    }
    virtual std::streamsize xsputn (const char_type *__s, std::streamsize __n) 
    {
        return write(__s, __n);
    }
    
    ///
    /// human readable Java like interface
    ///
    int read(void* dest, int size);
    int write(const void* data, int size);
};

/*
class zistream : public istream {
    zistream& open_file(char const* filename, ios_base::openmode mode = ios_base::in);
    zistream& open_socket(char*
};
*/
} // end namespace tinfra

#endif
