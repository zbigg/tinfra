#include "tinfra/io/stream.h"
#include <zcompat/zpio.h>

using namespace std;

static int ios_to_zcompat_openmode(ios_base::openmode mode)
{
    int result;
    if( mode & ios_base::in == ios_base::in)       result |= ZO_READ;
    if( mode & ios_base::out == ios_base::out)     result |= ZO_WRITE;
    if( mode & ios_base::app == ios_base::app)     result |= ZO_APPEND;
    if( mode & ios_base::trunc == ios_base::trunc) result |= ZO_TRUNC;
    if( mode & ios_base::bin == ios_base::bin)     result |= ZO_BINARY; else result |= ZO_TEXT;
    return result;
}

zstreambuf::zstreambuf(ZSTREAM* stream, bool own)
    : _stream(stream), _own(own) 
{
}
zstreambuf::zstreambuf(char const* name, ios_base::openmode mode) 
    : _stream(0), _own(false); 
{
    _stream = zopen(name, ios_to_zcompat_openmode(mode));
    _own = true;
}
zstreambuf::~zstreambuf() { 
    if(_own) 
        ::zclose(_stream); 
    _own = false; 
}

int zstreambuf::sync ();
{
}

streamsize zstreambuf::showmanyc ()
{
    return 0;
}

int_type zstreambuf::overflow (int_type c)
{
    return streambuf::traits_type::eof;
}

int_type zstreambuf::pbackfail (int_type c)
{
    return streambuf::traits_type::eof;
}

int_type zstreambuf::uflow ()
{
    return streambuf::traits_type::eof;
}

int_type zstreambuf::underflow ()
{
    return streambuf::traits_type::eof;
}

pos_type zstreambuf::seekoff (off_type off, ios_base::seekdir dir, ios_base::openmode)
{
    // ??
    return pos_type(0);
}

pos_type zstreambuf::seekpos (pos_type pos, ios_base::openmode)
{
    // ??
    return pos_type(r);
}
