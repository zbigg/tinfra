#include "safe_debug_print.h" // we implement this

#include <tinfra/fmt.h> // for tinfra::tprintf, TBD, to be removed
#include <cstdio>
#include <cstring>
#include <algorithm>

struct hex_digits {
    static char digit(int value) {
        switch( value ) {
        case 0: return '0';
        case 1: return '1';
        case 2: return '2';
        case 3: return '3';
        case 4: return '4';
        case 5: return '5';
        case 6: return '6';
        case 7: return '7';
        case 8: return '8';
        case 9: return '9';
        case 10: return 'a';
        case 11: return 'b';
        case 12: return 'c';
        case 13: return 'd';
        case 14: return 'e';
        case 15: return 'f';
        }
        return '?';
    }
};

struct dec_digits {
    static char digit(int value) {
        switch( value ) {
        case 0: return '0';
        case 1: return '1';
        case 2: return '2';
        case 3: return '3';
        case 4: return '4';
        case 5: return '5';
        case 6: return '6';
        case 7: return '7';
        case 8: return '8';
        case 9: return '9';
        }
        return '?';
    }
};

template <typename T, int base, typename digits_type>
static int unsigned_integer_to_string(T value, char* dest)
{
    if( value == 0 ) {
        *dest = '0';
        *(dest+1) = '\0';
        return 1;
    }

    char* begin = dest;
    while( value != 0 ) {
        T remainder = value % base;

        *dest = digits_type::digit(remainder);
        *(dest+1) = '\0';

        value = value / base;
        dest++;
    }

    std::reverse(begin, dest);
    return dest-begin;
}

template <typename T, int base, typename digits_type>
static int signed_integer_to_string(T value, char* dest)
{
    if( value == 0 ) {
        *dest = '0';
        *(dest+1) = '\0';
        return 1;
    }
    char* begin_orig = dest;
    T multiplier=1;
    if( value < 0 ) {
        *dest++ = '-';
        multiplier=-1;
    }
    char* begin_digits = dest;
    while( value != 0 ) {
        T remainder = value % base;

        *dest = digits_type::digit(remainder*multiplier);
        *(dest+1) = '\0';

        value = value / base;
        dest++;
    }

    std::reverse(begin_digits, dest);
    return dest-begin_orig;
}

template <typename T>
static int signed_integer_to_string_dec(T value, char* dest)
{
    return signed_integer_to_string<T, 10, dec_digits>(value,dest);
}


template <typename T>
static int unsigned_integer_to_string_dec(T value, char* dest)
{
    return unsigned_integer_to_string<T, 10, dec_digits>(value,dest);
}

template <typename T>
static int unsigned_integer_to_string_hex(T value, char* dest)
{
    return unsigned_integer_to_string<T, 16, hex_digits>(value,dest);
}

void tinfra_safe_debug_print(tinfra::output_stream& out, char const* v) {
    char buf[16];
    int len = signed_integer_to_string_dec(*v, buf);

    out.write(buf, len);
}

void tinfra_safe_debug_print(tinfra::output_stream& out, signed char const* v) {
    char buf[16];
    int len = signed_integer_to_string_dec(*v, buf);

    out.write(buf, len);
}

void tinfra_safe_debug_print(tinfra::output_stream& out, unsigned char const* v) {
    char buf[16];
    int len = unsigned_integer_to_string_dec(*v, buf);

    out.write(buf, len);
}

void tinfra_safe_debug_print(tinfra::output_stream& out, short const* v) {
    char buf[16];
    int len = signed_integer_to_string_dec(*v, buf);

    out.write(buf, len);
}

void tinfra_safe_debug_print(tinfra::output_stream& out, unsigned short const* v) {
    char buf[16];
    int len = unsigned_integer_to_string_dec(*v, buf);

    out.write(buf, len);
}

void tinfra_safe_debug_print(tinfra::output_stream& out, int const* v) {
    char buf[24];
    int len = signed_integer_to_string_dec(*v, buf);

    out.write(buf, len);
}


void tinfra_safe_debug_print(tinfra::output_stream& out, unsigned int const* v) {
    char buf[24];
    int len = unsigned_integer_to_string_dec(*v, buf);

    out.write(buf, len);
}

void tinfra_safe_debug_print(tinfra::output_stream& out, long const* v) {
    char buf[48];
    int len = signed_integer_to_string_dec(*v, buf);

    out.write(buf, len);
}

void tinfra_safe_debug_print(tinfra::output_stream& out, unsigned long const* v) {
    char buf[48];
    int len = unsigned_integer_to_string_dec(*v, buf);

    out.write(buf, len);
}

void tinfra_safe_debug_print(tinfra::output_stream& out, long long const* v) {
    char buf[48];
    int len = signed_integer_to_string_dec(*v, buf);

    out.write(buf, len);
}

void tinfra_safe_debug_print(tinfra::output_stream& out, unsigned long long const* v) {
    char buf[48];
    int len = unsigned_integer_to_string_dec(*v, buf);
    out.write(buf, len);
}

void tinfra_safe_debug_print(tinfra::output_stream& out, void const* const* v) {
    char buf[48] = {"0x"};
    int len = unsigned_integer_to_string_hex((long long)*v, buf+2);
    out.write(buf, len+2);
}

void tinfra_safe_debug_print(tinfra::output_stream& out, void* const* v) {
    char buf[48] = {"0x"};
    int len = unsigned_integer_to_string_hex((long long)*v, buf+2);
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
            unsigned_integer_to_string_hex(*i, buf+2);
            out.write(buf,strlen(buf));
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
            unsigned_integer_to_string_hex(*i, buf+2);
            out.write(buf, strlen(buf));
            ib = i+1;
        }
        i++;
    }
    if( i != ib ) out.write(ib, (i-ib));
}

