#ifndef tinfra_lex_h_included
#define tinfra_lex_h_included

#include "tstring.h"
#include "symbol.h"

#include <stdexcept>
#include <string>
#include <ostream>
#include <sstream>

namespace tinfra {

template <typename T>
struct default_string_format {
    static void to_string(T const& v, std::string& dest) { 
        std::ostringstream fmt;
        to_string(v, fmt);
        dest = fmt.str();
    }
    static void to_string(T const& v, std::ostream& dest) { 
        dest << v;
    }
    static void from_string(tstring const& v, T& dest) { 
            // TODO: istringstream doesn't have (ptr, len) constructor
        std::istringstream in(v.str());
        in >> dest; // TODO: catch IO, format failures
            
        //std::cerr << "from_string<" << TypeTraits<T>::name() << ">(" << v << ") -> " << dest << std::endl;
    }
};

// default implementation to catch all default casts
template <typename T>
struct string_format_traits: public default_string_format<T> {
};

// strings are can be casted with no-op
template<> 
struct string_format_traits<std::string> {
    static void to_string(std::string const& v, std::string& dest) {
        dest = v;
    }
    static void to_string(std::string const& v, std::ostream& dest) {
        dest << v;
    }
    static void from_string(tstring const& v, std::string& dest) {
        dest.assign(v.data(), v.size());
    }
    static void from_string(std::string& v, std::string& dest) {
        dest = v;
    }
};

template<> 
struct string_format_traits<const char*> {
    static void to_string(const char* v, std::string& dest) {
        dest = v;
    }
    static void to_string(const char* v, std::ostream& dest) {
        dest << v;
    }	
};

// strings are can be casted with no-op
template<> 
struct string_format_traits<tstring> {
    static void to_string(tstring const& v, std::string& dest) {
        dest.assign(v.data(), v.size());
    }
    static void to_string(tstring const& v, std::ostream& dest) {
        dest << v;
    }
};

template<> 
struct string_format_traits<char*> {
    static void to_string(const char* v, std::string& dest) {
        dest = v;
    }
    static void to_string(const char* v, std::ostream& dest) {
        dest << v;
    }	
};

template<int N> 
struct string_format_traits<char[N]> {
    static void to_string(const char v[N], std::string& dest) {
        dest = v;
    }
    static void to_string(const char v[N], std::ostream& dest) {
        dest << v;
    }
    static void from_string(tstring const& v, char dest[N]) {
        if( v.size() <= N-1 ) {
            std::memcpy(dest,v.data(), v.size());
            dest[v.size()] = 0;
        } else {
            throw std::logic_error("trying to assign too big string");
        }
    }
};

template<int N> 
struct string_format_traits<char const[N]> {
    static void to_string(const char v[N], std::string& dest) {
        dest = v;
    }
    static void to_string(const char v[N], std::ostream& dest) {
        dest << v;
    }
};

template<> 
struct string_format_traits<symbol> {
    static void to_string(symbol const& v, std::string& dest) {	    
        dest = v.str();
    }
    static void to_string(symbol const& v, std::ostream& dest) {
        dest << v.c_str();
    }
    static void from_string(tstring const& v, symbol& dest) {	    
        dest = symbol(v);
    }	
};

//
// core from_string, to_string
//

template<typename T>
void from_string(tstring const& str, T& dest) {
    string_format_traits<T>::from_string(str, dest);
}


template <typename T>
void to_string(T const& value, std::string& dest) {
    string_format_traits<T>::to_string(value, dest);
}

template <typename T>
void to_string(T const& value, std::ostream& dest) {
    string_format_traits<T>::to_string(value, dest);
}

//
// convienience versions of from_string, to_string
//

template<typename T>
T from_string(tstring const& str) {
    T result;
    from_string<T>(str, result);
    return result;
}

template <typename T>
std::string to_string(T const& value) {
    std::string result;
    to_string<T>(value, result);
    return result;
}
    
} // end namespace tinfra

#endif // tinfra_lex_h_included
