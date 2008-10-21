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
#include "tinfra/tinfra.h"
#ifdef _WINSOCK
#include "winsock.h"
#else
#include <arpa/inet.h>
#endif

namespace primitive_wrapper {
    
//
// types defined in RFC
//
template <typename T>
class integer_wrapper {
    T _v;
    
public:
    primitive_wrapper() {}
    primitive_wrapper(T v): _v(v) {}

    
    primitive_wrapper& operator=(T v) { _v = v; }
    
    operator T const&() const { return v; }
    operator T&() { return v; }
};

template <typename T> struct optimized_integer {
    typedef int type;
}

template<> struct optimized_integer<long long> { typedef long long type; }
template<> struct optimized_integer<unsigned long long> { typedef unsigned long long type; }

#define PRIMITIVE_INTEGER_WRAPPER(type, name)                 \
    typedef integer_wrapper<type> name;                       \
    std::ostream& operator <<(std::ostream& s, name ) {       \
        return s << static_cast<optimized_integer<type> >(v); \
    }

PRIMITIVE_INTEGER_WRAPPER(signed   char,      int8);
PRIMITIVE_INTEGER_WRAPPER(unsigned char,      uint8);
PRIMITIVE_INTEGER_WRAPPER(signed   short,     int16);
PRIMITIVE_INTEGER_WRAPPER(unsigned short,     uint16);
PRIMITIVE_INTEGER_WRAPPER(signed   int,       int32);
PRIMITIVE_INTEGER_WRAPPER(unsigned int,       uint32);
PRIMITIVE_INTEGER_WRAPPER(signed   long long, int64);
PRIMITIVE_INTEGER_WRAPPER(unsigned long long, uint64);

} // end namespace rfc4251
