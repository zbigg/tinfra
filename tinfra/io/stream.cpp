#include "tinfra/io/stream.h"
#include <sstream>

#include <zcompat/zpio.h>

using namespace std;

namespace tinfra {
    
static int ios_to_zcompat_openmode(ios::openmode mode)
{
    int result;
    if( (mode & ios::in) == ios::in)       result |= ZO_READ;
    if( (mode & ios::out) == ios::out)     result |= ZO_WRITE;
    if( (mode & ios::app) == ios::app)     result |= ZO_APPEND;
    if( (mode & ios::trunc) == ios::trunc) result |= ZO_TRUNC;
    //if( mode & ios::bin == ios::bin)     result |= ZO_BINARY;  else result |= ZO_TEXT;
    return result;
}

zstreambuf::zstreambuf(ZSTREAM stream, bool own)
    : _stream(stream), _own(own) 
{
}

zstreambuf::zstreambuf(char const* name, ios::openmode mode) 
    : _stream(0), _own(false)
{
    _stream = zopen(name, ios_to_zcompat_openmode(mode));
    _own = true;
}

zstreambuf::~zstreambuf() { 
    close();
}

zstreambuf& zstreambuf::open_file(char const* name, ios::openmode mode)
{
    close();
    _stream = zfopen(name,  ios_to_zcompat_openmode(mode));
    if( _stream == 0 ) {
        throw io_exception(string("unable to open '") + name + "': " + zstrerror(errno));
    }
    _own = true;
    return *this;
}

zstreambuf& zstreambuf::open_socket(char const* target, int port)
{
    close();
    _stream = zsopen(target, (unsigned short)port);
    if( _stream == 0 ) {
        ostringstream msg;
        msg << "unable to connect '" << target << ":" << port << ": " << zstrerror(errno);
        throw io_exception(msg.str());
    }
    _own = true;
    return *this;
}

void zstreambuf::close()
{
    if(_own) 
        ::zclose(_stream); 
    _stream = 0;
    _own = false; 
}

int zstreambuf::sync() {
    return 0;
}

streamsize zstreambuf::showmanyc ()
{
    return 0;
}

zstreambuf::int_type zstreambuf::overflow (int_type c)
{
    return streambuf::traits_type::eof();
}

zstreambuf::int_type zstreambuf::pbackfail (int_type c)
{
    return streambuf::traits_type::eof();
}

zstreambuf::int_type zstreambuf::uflow ()
{
    return streambuf::traits_type::eof();
}

zstreambuf::int_type zstreambuf::underflow ()
{
    return streambuf::traits_type::eof();
}

zstreambuf::pos_type zstreambuf::seekoff (off_type off, ios::seekdir dir, ios::openmode)
{
    // ??
    return pos_type(0);
}

zstreambuf::pos_type zstreambuf::seekpos (pos_type pos, ios::openmode)
{
    // ??
    return pos_type(pos);
}

int zstreambuf::read(void* dest, int size) {
    int result = ::zread(_stream, dest, size);
    if( result == -1 ) {
        throw io_exception(zstrerror(errno));
    }
    return result;
}

int zstreambuf::write(const void* data, int size) {
    int result = ::zwrite(_stream, data, size);
    if( result == -1 ) {
        throw io_exception(zstrerror(errno));
    }
    return result;
}

} //end namespace tinfra

