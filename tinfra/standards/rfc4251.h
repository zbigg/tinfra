//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_rfc4251_h__
#define __tinfra_rfc4251_h__

#include <string>
#include "tinfra/symbol.h"
#include "tinfra/tinfra.h"
#include "tinfra/io/stream.h"
#include "tinfra/fmt.h"

#ifdef _WINSOCK
#include "winsock.h"
#else
#include <arpa/inet.h>
#endif

#include "tinfra/primitive_wrapper.h"

namespace tinfra {
namespace rfc4251 {
    
//
// types defined in RFC
//
    
typedef tinfra::primitive_wrapper::uint8  byte;
typedef tinfra::primitive_wrapper::uint8  boolean;
typedef tinfra::primitive_wrapper::uint32 uint32;
typedef tinfra::primitive_wrapper::uint64 uint64;

typedef std::string        string;


typedef std::vector<std::string> name_list;

// missing typedefs
//  mpint - we don't need this yet and it's not trivial

//
// binary serialization for supported types
//

class reader {
public:
    reader(const char* data, int length):
        data(data),
        length(length),
        next(data),
        end(data+length)
    {}
    
    void operator()(tinfra::symbol const&, byte& v)      { v = read_octet(); }    
    //void operator()(tinfra::symbol const&, boolean& v)   { v = read_octet<boolean>(); }
    void operator()(tinfra::symbol const&, uint32& v)    { v = read_uint32(); }
    
    void operator()(tinfra::symbol const&, uint64& v)    { v = read_uint64(); }
    void operator()(tinfra::symbol const&, string& v)    { read_string(v); }
    
    void operator()(tinfra::symbol const&, name_list& v) { read_name_list(v); }    
    
protected:
    const char* data;
    size_t      length;
    const char* next;
    const char* end;

    byte read_octet() {
        byte const* tnext = reinterpret_cast<byte const*>(next);
        advance(1, "byte");
        return *(tnext);
    }
    
    uint32   read_uint32() {
        uint32 const* tnext = reinterpret_cast<uint32 const*>(next);
        advance(4, "unit32");
        return ntohl( tnext[0] );
    }
    
    uint64   read_uint64() {
        uint32 const* tnext = reinterpret_cast<uint32 const*>(next);
        advance(8, "unit64");
        uint64_t hi = ntohl( tnext[0] );
        uint64_t lo = ntohl( tnext[1] );
        return  (hi << 32) | lo ;
    }
    
    void read_string(string& r)
    {
        size_t size = read_uint32();
        read_string(size, r);
    }
    
    void read_string(size_t length, string& r)
    {
        const char* tnext = next;
        advance(length, "string length");
        r.assign(tnext, length);
    }
    
    void read_name_list(name_list& r)
    {
        std::string bytes;
        read_string(bytes);
        
        r = tinfra::split(bytes, ","); // TODO: this could be optimized
    }
    
    size_t readed() const 
    {
        return (next - data);
    }
    
    void advance(int n, const char* what)
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
            //
            // THROW_ANALYSIS: escape condition, not available
            throw tinfra::io::would_block(tinfra::fmt(
                "not enough information to assemble message, wanted %i bytes for %s at position %i")
                % n % what % (next-data));
        }
    }
};


class writer {
    std::string& out;

public:    
    writer(std::string& out): out(out) {}
    
    void operator()(tinfra::symbol const&, byte v)             { write_octet<byte>(v); }    
    //void operator()(tinfra::symbol const&, boolean v)          { v = write_octet<boolean>(v); }
    void operator()(tinfra::symbol const&, uint32 v)           { write_uint32(v); }
    
    void operator()(tinfra::symbol const&, uint64 v)           { write_uint64(v); }
    void operator()(tinfra::symbol const&, string const& v)    { write_string(v); }
    
    void operator()(tinfra::symbol const&, name_list const& v) { write_name_list(v); }  
        
    const std::string& str() { return out; }
    
protected:
    
    template <typename T>
    void write_octet(T v) {
        write<T>(static_cast<unsigned char>(v));
    }
    
    void write_uint32(uint32 v) {
        write<uint32>(htonl(v));
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
        for( name_list::const_iterator i = nl.begin(); i != nl.end(); ++i ) {
            if( i != nl.begin() )
                len += 1;
            
            len += i->size();            
        }
        
        std::string tmp;
        tmp.reserve(len);
        for( name_list::const_iterator i = nl.begin(); i != nl.end(); ++i ) {
            if( i != nl.begin() )
                tmp.append(1, ',');
            
            tmp.append(*i);          
        }
        
        write_string(tmp);
    }
    
    template <typename T>
    void write(T const& v) {
        write(reinterpret_cast<const char*>(&v), sizeof(v));
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



}} // end namespace rfc4251

#endif __tinfra_rfc4251_h__
