//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include <stdexcept>
#include <string>
#include "tinfra/io/stream.h"
#include "tinfra/symbol.h"

namespace rfc4251 {
    
//
// types defined in RFC
//
    
typedef unsigned char  byte;
typedef unsigned char  boolean;
typedef uint32_t       uint32;
typedef uint64_t       uint64;

typedef std::string    string;


typedef std::vector<std::string> name_list;

// missing typedefs
//  mpint - we don't need this yet and it's not trivial

//    
// packet types
//


//
// binary serialization for supported types
//

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
    
    void operator(tinfra::symbol const&, byte& v)      { v = read_octet<byte>(); }    
    void operator(tinfra::symbol const&, boolean& v)   { v = read_octet<boolean>(); }
    void operator(tinfra::symbol const&, uint32& v)    { v = read_uint32(); }
    
    void operator(tinfra::symbol const&, uint64& v)    { v = read_uint64(); }
    void operator(tinfra::symbol const&, string& v)    { v = read_string(v); }
    
    void operator(tinfra::symbol const&, name_list& v) { v = read_name_list(v); }    
protected:
    template <typename T>
    T read_octet() {
        T const* tnext = static_cast<T const*>(next);
        advance(1);
        return *(tnext);
    }
    
    template <typename T>
    uint32   read_uint32() {
        unsigned int const* tnext = static_cast<const unsigned int*>(next);
        advance(4);
        return ntohl( tnext[0] );
    }
    
    uint64   read_uint64() {
        unsigned int const* tnext = static_cast<const unsigned int*>(next);
        advance(8);
        uint64_t hi = ntohl( tnext[0] );
        uint64_t lo = ntohl( tnext[1] );
        return  (hi << 32) | lo ;
    }
    
    void read_string(string& r)
    {
        size_t size = read_uint32();
        return string(size, r);
    }
    
    void read_string(size_t length, string& r)
    {
        const char* tnext = next;
        advance(length);
        return r.assign(tnext, length);
    }
    
    void read_name_list(name_list& r)
    {
        std::string bytes;
        read_string(bytes);
        
        r = tinfra::split(bytes, ","); // TODO: this could be optimized
    }
    
    size_t readed() const
        return (next - begin);
    }
    
protected:
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
            // TODO should create option to have reader reporting error
            //      - namely std::logic_error with something like expecting 4 bytes
            //        or message lengh should be at least next + bytes
            throw tinfra::would_block("not enough information to assemble message");
        }
    }
};


class writer {
    std::string& out;

public:    
    writer(std::string& out): out(out) {}
    
    void operator(tinfra::symbol const&, byte v)             { write_octet<byte>(v); }    
    void operator(tinfra::symbol const&, boolean v)          { v = write_octet<boolean>(v); }
    void operator(tinfra::symbol const&, uint32 v)           { v = write_uint32(v); }
    
    void operator(tinfra::symbol const&, uint64 v)           { v = write_uint64(v); }
    void operator(tinfra::symbol const&, string const& v)    { v = write_string(v); }
    
    void operator(tinfra::symbol const&, name_list const& v) { v = write_name_list(v); }  
        
    const std::string& str() { return out; }
    
protected:
    
    template <typename T>
    void write_octet(T v) {
        write<T>(static_cast<unsigned char>(v));
    }
    
    void write_uint32(uint32 v) {
        write<uint32>(htons(v));
    }
    
    void write_uint64(uint64 v) {
        uint32 lo = v &  0xFFFFFFFF;
        uint32 hi = v >> 32;
        write<uint32>(htonl(hi));
        write<uint32>(htonl(lo));
    }
    
    void write_string(string const& s)
    {
        write_uint32(s.size());
        write(s.data(), s.size());
    }
    
    void write_name_list(name_list const& nl)
    {
        size_t len = 0;
        for( name_list::const_iterator& i = nl.begin(); i != nl.end(); ++i ) {
            if( i != nl.begin() )
                nl += 1;
            
            nl += i->size();            
        }
        
        std::string tmp;
        tmp.reserve(len);
        for( name_list::const_iterator& i = nl.begin(); i != nl.end(); ++i ) {
            if( i != nl.begin() )
                tmp.append(1, ',');
            
            tmp.append(*i);          
        }
        
        write_string(tmp);
    }
    
    template <typename T>
    void write(T const& v) {
        write(static_cast<const char*)(&v), sizeof(v));
    }
    
    void write(const char* data, size_t size)
    {
        out.append(data, size);
    }
};

template <typename T>
std::string serialize(T const& value)
{
    string buffer;
    writer processor(buffer);    
    tinfra::process(value, processor);
    
    return buffer;
}

template <typename T>
void deserialize(std::string const& buffer, T& dest)
{
    reader processor(buffer.data(), buffer.size());
    
    tinfra::mutate(dest, processor);
}

} // end namespace rfc4251
