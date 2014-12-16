#ifndef tinfra_basic_int_to_string_h_included
#define tinfra_basic_int_to_string_h_included

#include "platform.h" // TINFRA_UNLIKELY

#include <algorithm> // std::reverse

namespace tinfra {

//
// interface
//   

template <typename T>
int signed_integer_to_string_dec(T value, char* dest, size_t buf_size);

template <typename T>
int unsigned_integer_to_string_dec(T value, char* dest, size_t buf_size);

template <typename T>
int unsigned_integer_to_string_hex(T value, char* dest, size_t buf_size);


//
// implementaion 
//   fully templated

namespace detail {

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
int unsigned_integer_to_string(T value, char* dest, size_t buf_size)
{
    if( buf_size < 2 ) {
        return -1;
    }
    if( value == 0 ) {
        *dest = '0';
        *(dest+1) = '\0';
        return 1;
    }
    size_t pos = 0;
    char* begin = dest;
    while( value != 0 ) {
        T remainder = value % base;
        if( TINFRA_UNLIKELY(pos >= buf_size-1) ) {
            return -1;
        }
        *dest = digits_type::digit(remainder);
        *(dest+1) = '\0';

        value = value / base;
        dest++;
        pos++;
    }

    std::reverse(begin, dest);
    return dest-begin;
}

template <typename T, int base, typename digits_type>
int signed_integer_to_string(T value, char* dest, size_t buf_size)
{
    if( buf_size < 2 ) {
        return -1;
    }
    if( value == 0 ) {
        *dest = '0';
        *(dest+1) = '\0';
        return 1;
    }
    size_t pos = 0;
    char* begin_orig = dest;
    T multiplier=1;
    if( value < 0 ) {
        *dest++ = '-';
        multiplier=-1;
    }
    char* begin_digits = dest;
    while( value != 0 ) {
        T remainder = value % base;
        if( TINFRA_UNLIKELY(pos >= buf_size-1) ) {
            return -1;
        }
        *dest = digits_type::digit(remainder*multiplier);
        *(dest+1) = '\0';

        value = value / base;
        dest++;
        pos++;
    }

    std::reverse(begin_digits, dest);
    return dest-begin_orig;
}

} // end namespace detail (now ::tinfra )

template <typename T>
int signed_integer_to_string_dec(T value, char* dest, size_t buf_size)
{
    return detail::signed_integer_to_string<T, 10, detail::dec_digits>(value, dest, buf_size);
}

template <typename T>
int unsigned_integer_to_string_dec(T value, char* dest, size_t buf_size)
{
    return detail::unsigned_integer_to_string<T, 10, detail::dec_digits>(value, dest, buf_size);
}

template <typename T>
int unsigned_integer_to_string_hex(T value, char* dest, size_t buf_size)
{
    return detail::unsigned_integer_to_string<T, 16, detail::hex_digits>(value, dest, buf_size);
}

} // end namespace tinfra

#endif //tinfra_basic_int_to_string_h_included
