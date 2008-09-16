//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_lex_h__
#define __tinfra_lex_h__
#include <sstream>

#include "tinfra/platform.h"
#include "tinfra.h"
#include "exception.h"

namespace tinfra {

class bad_lexical_cast: public generic_exception {
public:
    bad_lexical_cast(const char* src, const char* dest): 
	generic_exception(std::string("cannot cast from '") + std::string(src) + std::string("' to '") + std::string(dest) + "'")
    {}
    virtual ~bad_lexical_cast() throw() {}
};
namespace detail {
	template <typename T>
	struct DefaultLexicalInterpreter {
		static void to_string(T const& v, std::string& dest) { 
			std::ostringstream fmt;
			to_string(v, fmt);
			dest = fmt.str();
		}
		static void to_string(T const& v, std::ostream& dest) { 
			dest << v;
		}
		static void from_string(const char* v, T& dest) { 
			std::istringstream in(v);
			in >> dest; // TODO: catch IO, format failures
			
			//std::cerr << "from_string<" << TypeTraits<T>::name() << ">(" << v << ") -> " << dest << std::endl;
		}
	};
};

// default implementation to catch all default casts
template <typename T>
struct LexicalInterpreter {
	static void to_string(T const&, std::string&);
	static void to_string(T const&, std::ostream&);
	static void from_string(const char*, T&);
};

// default implementation provided by default stream bases string IO
template<> struct LexicalInterpreter<int>: public detail::DefaultLexicalInterpreter<int> {};
template<> struct LexicalInterpreter<unsigned int>: public detail::DefaultLexicalInterpreter<unsigned int> {};

template<> struct LexicalInterpreter<char>: public detail::DefaultLexicalInterpreter<char> {};
template<> struct LexicalInterpreter<signed char>: public detail::DefaultLexicalInterpreter<signed char> {};
template<> struct LexicalInterpreter<unsigned char>: public detail::DefaultLexicalInterpreter<unsigned char> {};

template<> struct LexicalInterpreter<short>: public detail::DefaultLexicalInterpreter<short> {};
template<> struct LexicalInterpreter<unsigned short>: public detail::DefaultLexicalInterpreter<unsigned short> {};

template<> struct LexicalInterpreter<long>: public detail::DefaultLexicalInterpreter<long> {};
template<> struct LexicalInterpreter<unsigned long>: public detail::DefaultLexicalInterpreter<unsigned long> {};
    
template<> struct LexicalInterpreter<float>: public detail::DefaultLexicalInterpreter<float> {};    
template<> struct LexicalInterpreter<double>: public detail::DefaultLexicalInterpreter<double> {};

// strings are can be casted with no-op
template<> 
struct LexicalInterpreter<std::string> {
	static void to_string(std::string const& v, std::string& dest) {
		dest = v;
	}
	static void to_string(std::string const& v, std::ostream& dest) {
		dest << v;
	}
	static void from_string(const char* v, std::string& dest) {
		dest = v;
	}
};

template<> 
struct LexicalInterpreter<const char*> {
	static void to_string(const char* v, std::string& dest) {
		dest = v;
	}
	static void to_string(const char* v, std::ostream& dest) {
		dest << v;
	}	
};

template<> 
struct LexicalInterpreter<char*> {
	static void to_string(const char* v, std::string& dest) {
		dest = v;
	}
	static void to_string(const char* v, std::ostream& dest) {
		dest << v;
	}	
};

template<int N> 
struct LexicalInterpreter<char[N]> {
	static void to_string(const char v[N], std::string& dest) {
		dest = v;
	}
	static void to_string(const char v[N], std::ostream& dest) {
		dest << v;
	}
	static void from_string(const char* v, char dest[N]) {
		if( ::strlen(v) <= N-1 ) {
		    ::strcpy(dest,v);
		} else {
            throw bad_lexical_cast(TypeTraits<const char*>::name(), TypeTraits<char[N]>::name());
		}
	}
};

template<int N> 
struct LexicalInterpreter<char const[N]> {
	static void to_string(const char v[N], std::string& dest) {
		dest = v;
	}
	static void to_string(const char v[N], std::ostream& dest) {
		dest << v;
	}
};

template<> 
struct LexicalInterpreter<symbol> {
	static void to_string(symbol const& v, std::string& dest) {	    
	    dest = v.str();
	}
	static void to_string(symbol const& v, std::ostream& dest) {
	    dest << v.c_str();
	}
	static void from_string(const char* v, symbol& dest) {	    
	    dest = symbol(v);
	}	
};
//
//
//

namespace detail {    
	class LexicalSetter {
		symbol      _field;
		char const* _value;
	public:		
		LexicalSetter(symbol field,char const* value): _field(field), _value(value) {}
		
		template <typename F>
		void operator ()(symbol const& symbol, F& v) {
			if( symbol == _field ) LexicalInterpreter<F>::from_string(_value, v);
		}
	    
	};

	class LexicalGetter {
	public:
		symbol       _field;
		std::string& _dest;		
	public:
		LexicalGetter(symbol field, std::string& dest): _field(field), _dest(dest) {}
		
		template <class F>
		void operator ()(symbol const& symbol, F const& v) {
			if( symbol == _field ) LexicalInterpreter<F>::to_string(v, _dest);
		}    
	};
} // end of details

///
/// lexical get & set
///

template<typename T>
void lexical_set(T& obj, symbol field, char const* value)
{
	detail::LexicalSetter setter(field,value);
	mutate(obj, setter);
}
template<typename T>
void lexical_set(T& obj, symbol field, std::string const& value)
{
	detail::LexicalSetter setter(field,value.c_str());
	mutate(obj, setter);
}

template<typename T>
void lexical_get(T const& obj, symbol field, std::string& dest)
{
	detail::LexicalGetter getter(field,dest);
	process(obj, getter);
}

template<typename T>
std::string lexical_get(T const& obj, symbol field)
{
	std::string dest;
	detail::LexicalGetter getter(field,dest);
	process(obj, getter);
	return dest;
}

template<typename F>
void from_string(char const* str, F& dest) {
        LexicalInterpreter<F>::from_string(str, dest);
}

template<typename F>
void from_string(std::string const& str, F& dest) {
        LexicalInterpreter<F>::from_string(str.c_str(), dest);
}


template <typename F>
void to_string(F const& value, std::string& dest) {
	LexicalInterpreter<F>::to_string(value, dest);
}

template <typename F>
void to_string(F const& value, std::ostream& dest) {
	LexicalInterpreter<F>::to_string(value, dest);
}

} // nemaspace tinfra

#endif // __tinfra_lex_h__
