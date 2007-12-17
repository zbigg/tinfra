#ifdef __tinfra_io_stream_h_
#define __tinfra_io_stream_h_

#include <zcompat/zpio.h>

namespace tinfra { 
    
class zstreambuf : public streambuf {
private:
    ZSTREAM*    _stream;
    bool        _own;
public:
    zstreambuf(ZSTREAM* stream = 0, bool own = false);
    zstreambuf(char const* name, ios_base::openmode mode = ios_base::in);

    /** Open a file in native filesystem */
    zstreambuf& open_file(char const* filename, ios_base::openmode mode = ios_base::in);

    /** Opens a client tcp connection. */
    zstreambuf& open_socket(char const* target, int port);

    /** Opens a pipe to command.

        Spawns a 'command' with stdout and/or stdin hijacked by this streambuf.
    */
    zstreambuf& open_pipe(char const* command, ios_base::openmode mode = ios_base::in);

    /** Opens an anonymous pipe

        That pipe can be used in-process to communicate between threads.
    */
    zstreambuf& open_pipe();
    virtual ~zstreambuf();
    
    virtual void imbue (const locale &)
    {
    }
    virtual int_type overflow (int_type=traits_type::eof());
    virtual int_type pbackfail (int_type=traits_type::eof());
    virtual pos_type seekoff (off_type, ios_base::seekdir, ios_base::openmode=ios_base::in|ios_base::out);
    virtual pos_type seekpos (pos_type, ios_base::openmode=ios_base::in|ios_base::out);    
    //virtual basic_streambuf< char_type, _Traits >* setbuf (char_type *, streamsize)    
    virtual streamsize showmanyc ();
    virtual int sync ();
    virtual int_type uflow ();
    virtual int_type underflow ();
    
    virtual streamsize xsgetn (char_type *__s, streamsize __n)
    {
        return ::zread(_stream, __s, __n);
    }
    virtual streamsize xsputn (const char_type *__s, streamsize __n) 
    {
        return ::zwrite(_stream, __s, _n);
    }
};
/*
class zistream : public istream {
    zistream& open_file(char const* filename, ios_base::openmode mode = ios_base::in);
    zistream& open_socket(char*
};
*/
} // end namespace tinfra

#endif