//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include <stdexcept>
#include <string>

#include "tinfra/io/stream.h"
#include "tinfra/symbol.h"

#include "message_raw.h"

namespace network_serializer {

class reader {
    const char* data;
    size_t      length;
    const char* next;
    const char* end;
public:
    reader(const char* data, int length):
        data(data),
        length(length),
        next(data),
        end(data+length)
    {}
    
    void operator(tinfra::symbol const&, unsigned char& v)  { v = octet<unsigned char>(); }    
    void operator(tinfra::symbol const&, char& v)           { v = octet<char>(); }
    void operator(tinfra::symbol const&, signed char& v)    { v = octet<signed char>(); }
    
    void operator(tinfra::symbol const&,          short& v) { v = int16<short>(); }
    void operator(tinfra::symbol const&, unsigned short& v) { v = int16<unsigned short>(); }
    
    void operator(tinfra::symbol const&,          int& v)   { v = int32<int>(); }
    void operator(tinfra::symbol const&, unsigned int& v)   { v = int32<unsigned int>(); }
    
    void operator(tinfra::symbol const&, std::string& v)    { v = string(); }
    
protected:
    template <typename T>
    T octet() {
        T const* tnext = static_cast<T const*>(next);
        advance(1);
        return *(tnext);
    }
    
    template <typename T>
    T int16() {
        const unsigned short* tnext = static_cast<const unsigned short*>(next);
        advance(2);
        return static_cast<T>( ntohs( *tnext ) );
    }
    
    template <typename T>
    T   int32() {
        unsigned int const* tnext = static_cast<const unsigned int*>(next);
        advance(4);
        return (T) ntohl( * tnext );
    }
    
    std::string    string()
    {
        size_t size = uint32();
        return string(size);
    }
    
    std::string    string(size_t length)
    {
        const char* tnext = next;
        advance(length);
        return std::string(tnext, length);
    }
    
    size_t readed() const
        return (next - begin);
    }
private:
    void advance(int n)
    {
        if( next + n <= end ) {
            next += n;
            return;
        }
        else {
            // 1. ha it's bad design, we shouldn't throw on normal situation
            //    here normal is that due to network property we have received
            //    partial message
            // but how often you'll receive broken packets ?
            throw tinfra::would_block("not enough information to assemble message");
        }
    }
};

class writer {
    std::string& out;

public:    
    writer(std::string& out): out(out) {}
    
    void operator(tinfra::symbol const&, unsigned char v)  { octet<unsigned char>(v); }    
    void operator(tinfra::symbol const&, char v)           { octet<char>(v); }
    void operator(tinfra::symbol const&, signed char v)    { octet<signed char>(v); }
    
    void operator(tinfra::symbol const&,          short v) { int16<short>(v); }
    void operator(tinfra::symbol const&, unsigned short v) { int16<unsigned short>(v); }
    
    void operator(tinfra::symbol const&,          int v)   { int32<int>(v); }
    void operator(tinfra::symbol const&, unsigned int v)   { int32<unsigned int>(v); }
    
    void operator(tinfra::symbol const&, std::string const& v)    { string(v); }
    
    const std::string& str() { return out; }
    
protected:
    
    template <typename T>
    void octet(T v) {
        write<T>(static_cast<unsigned char>(v));
    }
    
    template <typename T>
    void int16(T v) {
        write<T>(htons(v));
    }
    
    template <typename T>
    void int32(T v) {
        write<T>(htonl(v));
    }
    
    void string(std::string const& s)
    {
        int32(s.size());
        write(s.data(), s.size());
    }
    
    template <typename T>
    void write(T v) {
        write(static_cast<const char*)(&v), sizeof(v));
    }
    
    void write(const char* data, size_t size)
    {
        out.append(data, size);
    }
};

} // end namespace network_serializer