#include <zcompat/zpiocpp.h>

namespace zcompat { 
    
template<typename _CharT, typename _Traits>
class basic_zstreambuf : public basic_streambuf<_CharT, _Traits> {
public:
    typedef _CharT 					char_type;
    typedef _Traits 					traits_type;
    typedef _Alloc				       	allocator_type;
    typedef typename traits_type::int_type 		int_type;
    typedef typename traits_type::pos_type 		pos_type;
    typedef typename traits_type::off_type 		off_type;

private:
    ZSTREAM*    _stream;
    bool        _own;
public:
    basic_zstreambuf(ZSTREAM* stream, bool own = false) : _stream(stream), _own(own) {}
    basic_zstreambuf(char const* name, ios_base::openmode mode = ios_base::in) {
        _stream = zopen(name, to_zcompat_openmode(mode));
        _own = true;
    }
    virtual ~basic_zstreambuf() { if(_own) zclose(_stream) ; _own = false; }
    
    virtual void imbue (const locale &)
    {
    }
    virtual int_type overflow (int_type=traits_type::eof())
    {
    }
    virtual int_type pbackfail (int_type=traits_type::eof())
    {
    }
    virtual pos_type seekoff (off_type, ios_base::seekdir, ios_base::openmode=ios_base::in|ios_base::out)
    {
    }
    virtual pos_type seekpos (pos_type, ios_base::openmode=ios_base::in|ios_base::out)
    {
    }
    virtual basic_streambuf< char_type, _Traits >* setbuf (char_type *, streamsize)
    {
    }
    virtual streamsize showmanyc ()
    {
    }
    virtual int sync ()
    {
    }
    virtual int_type uflow ()
    {
    }
    virtual int_type underflow ()
    {
    }
    virtual streamsize xsgetn (char_type *__s, streamsize __n)
    {
        return zread(_stream, __s, __n);
    }
    virtual streamsize xsputn (const char_type *__s, streamsize __n) 
    {
        return zwrite(_stream, __s, _n);
    }
private:
    int to_zcompat_openmode(ios_base::openmode mode)
    {
        int result;
        if( mode & ios_base::in == ios_base::in)       result |= ZO_READ;
        if( mode & ios_base::out == ios_base::out)     result |= ZO_WRITE;
        if( mode & ios_base::app == ios_base::app)     result |= ZO_APPEND;
        if( mode & ios_base::trunc == ios_base::trunc) result |= ZO_TRUNC;
        if( mode & ios_base::bin == ios_base::bin)     result |= ZO_BINARY; else result |= ZO_TEXT;
        return result;
    }
}

}