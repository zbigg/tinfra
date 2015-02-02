#include "safe_debug_print.h" // we implement this

#include "basic_int_to_string.h" // for [un]signed_integer_to_string_{dec|hex}

#include "assert.h"

#include <cstdio>
#include <cstring>
#include <cctype>
#include <algorithm>

#include "stdint.h"

using tinfra::signed_integer_to_string_dec;
using tinfra::unsigned_integer_to_string_dec;
using tinfra::unsigned_integer_to_string_hex;

void tinfra_safe_debug_print(tinfra::output_stream& out, char const* v) {
    char buf[16];
    const int len = signed_integer_to_string_dec(*v, buf, sizeof(buf));
    TINFRA_ASSERT(len > 0);
    out.write(buf, len);
}

void tinfra_safe_debug_print(tinfra::output_stream& out, signed char const* v) {
    char buf[16];
    const int len = signed_integer_to_string_dec(*v, buf, sizeof(buf));
    TINFRA_ASSERT(len > 0);
    out.write(buf, len);
}

void tinfra_safe_debug_print(tinfra::output_stream& out, unsigned char const* v) {
    char buf[16];
    const int len = unsigned_integer_to_string_dec(*v, buf, sizeof(buf));
    TINFRA_ASSERT(len > 0);
    out.write(buf, len);
}

void tinfra_safe_debug_print(tinfra::output_stream& out, short const* v) {
    char buf[16];
    const int len = signed_integer_to_string_dec(*v, buf, sizeof(buf));
    TINFRA_ASSERT(len > 0);
    out.write(buf, len);
}

void tinfra_safe_debug_print(tinfra::output_stream& out, unsigned short const* v) {
    char buf[16];
    const int len = unsigned_integer_to_string_dec(*v, buf, sizeof(buf));
    TINFRA_ASSERT(len > 0);
    out.write(buf, len);
}

void tinfra_safe_debug_print(tinfra::output_stream& out, int const* v) {
    char buf[24];
    const int len = signed_integer_to_string_dec(*v, buf, sizeof(buf));
    TINFRA_ASSERT(len > 0);
    out.write(buf, len);
}


void tinfra_safe_debug_print(tinfra::output_stream& out, unsigned int const* v) {
    char buf[24];
    const int len = unsigned_integer_to_string_dec(*v, buf, sizeof(buf));
    TINFRA_ASSERT(len > 0);
    out.write(buf, len);
}

void tinfra_safe_debug_print(tinfra::output_stream& out, long const* v) {
    char buf[48];
    const int len = signed_integer_to_string_dec(*v, buf, sizeof(buf));
    TINFRA_ASSERT(len > 0);
    out.write(buf, len);
}

void tinfra_safe_debug_print(tinfra::output_stream& out, unsigned long const* v) {
    char buf[48];
    const int len = unsigned_integer_to_string_dec(*v, buf, sizeof(buf));
    TINFRA_ASSERT(len > 0);
    out.write(buf, len);
}

void tinfra_safe_debug_print(tinfra::output_stream& out, long long const* v) {
    char buf[48];
    const int len = signed_integer_to_string_dec(*v, buf, sizeof(buf));
    TINFRA_ASSERT(len > 0);
    out.write(buf, len);
}

void tinfra_safe_debug_print(tinfra::output_stream& out, unsigned long long const* v) {
    char buf[48];
    const int len = unsigned_integer_to_string_dec(*v, buf, sizeof(buf));
    TINFRA_ASSERT(len > 0);
    out.write(buf, len);
}

void tinfra_safe_debug_print(tinfra::output_stream& out, void const* const* v) {
    char buf[48] = {"0x"};
    uintptr_t vi = (uintptr_t)*v;
    const int len = unsigned_integer_to_string_hex(vi, buf+2, sizeof(buf)-2);
    TINFRA_ASSERT(len > 0);
    out.write(buf, len+2);
}

void tinfra_safe_debug_print(tinfra::output_stream& out, void* const* v) {
    char buf[48] = {"0x"};
    uintptr_t vi = (uintptr_t)*v;
    const int len = unsigned_integer_to_string_hex(vi, buf+2, sizeof(buf)-2);
    TINFRA_ASSERT(len > 0);
    out.write(buf, len+2);
}

void tinfra_safe_debug_print(tinfra::output_stream& out, const char** foo)
{
    const char* i = *foo;
    const char* ib = i;
    while( *i ) {
        if( *i == '\n' ) {
            if( i != ib ) out.write(ib, (i-ib));

            out.write("\\n",2);
            ib = i+1;
        } else if( *i == '\r' ) {
            if( i != ib ) out.write(ib, (i-ib));

            out.write("\\r",2);
            ib = i+1;
        } else if( !std::isprint(*i) ) {
            if( i != ib ) out.write(ib, (i-ib));

            char buf[20] = "\\x";
            const int len = unsigned_integer_to_string_hex(*i, buf+2, sizeof(buf)-2);
            TINFRA_ASSERT(len > 0);

            out.write(buf, len + 2);
            ib = i+1;
        }
        i++;
    }
    if( i != ib ) out.write(ib, (i-ib));
}

template<>
void tinfra_safe_debug_print<std::string>(tinfra::output_stream& out, std::string const* foo)
{
    const char* i = foo->data();
    const char* e = foo->data() + foo->size();
    const char* ib = i;
    while( i < e ) {
        if( *i == '\n' ) {
            if( i != ib ) out.write(ib, (i-ib));

            out.write("\\n",2);
            ib = i+1;
        } else if( *i == '\r' ) {
            if( i != ib ) out.write(ib, (i-ib));

            out.write("\\r",2);
            ib = i+1;
        } else if( !std::isprint(*i) ) {
            if( i != ib ) out.write(ib, (i-ib));

            char buf[20] = "\\x";
            const int len = unsigned_integer_to_string_hex(*i, buf+2, sizeof(buf)-2);
            TINFRA_ASSERT(len > 0);
            out.write(buf, len + 2);
            ib = i+1;
        }
        i++;
    }
    if( i != ib ) out.write(ib, (i-ib));
}

