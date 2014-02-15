#ifndef tinfra_safe_debug_print_h_included
#define tinfra_safe_debug_print_h_included

#include <string>

#include "tinfra/stream.h"

namespace tinfra {

///
/// interface
///

/// safely print object value
///
/// Prints debug oriented text version of object.
///
template <typename T>
void safe_debug_print(tinfra::output_stream& out, T const& var);

typedef void (*safe_debug_print_func)(tinfra::output_stream& out, const void* object);

/// get printer function
///
/// get printer function for giver type
template <typename T>
safe_debug_print_func make_safe_debug_print_func(T const&);

} // end namespace tinfra

///
/// type designer interface
/// 
/// to provide implementaiton of safe_debug_print one shall
/// for standard types:
/// provide orverload for:
///
///   void ::tinfra_safe_debug_print(tinfra::output_stream& out, TYPE const* obj);
///
/// Implementations shall use only OS write interface
/// and C/C++ basic memory operations on already allocated
/// buffers.
/// No i/o, locking, memory allocation/dealocation is allowed
/// as safe_debug_print functions are designed to be called
/// in unsafe environment like "on-exception/on-abort/on-sigsegv
/// handlers.

template <typename T>
void tinfra_safe_debug_print(tinfra::output_stream& out, T const* foo);

void tinfra_safe_debug_print(tinfra::output_stream& out, char const*);
void tinfra_safe_debug_print(tinfra::output_stream& out, signed char const*);
void tinfra_safe_debug_print(tinfra::output_stream& out, unsigned char const*);
void tinfra_safe_debug_print(tinfra::output_stream& out, short const*);
void tinfra_safe_debug_print(tinfra::output_stream& out, unsigned short const*);
void tinfra_safe_debug_print(tinfra::output_stream& out, int const*);
void tinfra_safe_debug_print(tinfra::output_stream& out, unsigned int const*);
void tinfra_safe_debug_print(tinfra::output_stream& out, long const*);
void tinfra_safe_debug_print(tinfra::output_stream& out, unsigned long const*);
void tinfra_safe_debug_print(tinfra::output_stream& out, long long const*);
void tinfra_safe_debug_print(tinfra::output_stream& out, unsigned long long const*);
void tinfra_safe_debug_print(tinfra::output_stream& out, void* const*);
void tinfra_safe_debug_print(tinfra::output_stream& out, void const* const*);
void tinfra_safe_debug_print(tinfra::output_stream& out, const char**);
void tinfra_safe_debug_print(tinfra::output_stream& out, std::string const*);

//
// implementation
//

namespace tinfra {

template <typename T>
struct safe_debug_printer {
	static void print(tinfra::output_stream& out, const void* obj)
	{
		T const* obj2 = reinterpret_cast<T const*>(obj);
		::tinfra_safe_debug_print(out, obj2);
	}
};

template <>
struct safe_debug_printer<const char*> {
	static void print(tinfra::output_stream& out, const void* obj)
	{
		::tinfra_safe_debug_print(out, (const char**)obj);
	}
};

template <int N>
struct safe_debug_printer<char const[N]> {
	static void print(tinfra::output_stream& out, const void* obj)
	{
	    const char* f = (const char*)obj;
		::tinfra_safe_debug_print(out, &f);
	}
};

template <int N>
struct safe_debug_printer<char[N]> {
	static void print(tinfra::output_stream& out, const void* obj)
	{
		const char* f = (const char*)obj;
		::tinfra_safe_debug_print(out, &f);
	}
};

template <typename T>
safe_debug_print_func make_safe_debug_print_func(T const&)
{
	return &tinfra::safe_debug_printer<T>::print;
}

template <typename T>
void safe_debug_print(tinfra::output_stream& out, T const& var)
{
    tinfra::safe_debug_printer<T>::print(out, &var);
}

} // end namespace tinfra

#endif // tinfra_safe_debug_print_h_included

