#ifndef get_exception_h_included
#define get_exception_h_included

#include <iostream>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <vector>
#include <typeinfo>

class execution_exception: public std::runtime_error {
public:
	execution_exception(std::string const& msg): std::runtime_error(msg) {}
};

typedef std::string (* any_to_string_t)(void*);

void register_exception_type(std::type_info const& ti, any_to_string_t converer);

template <typename T, typename P>
void register_exception_type(std::string (*converter)(P const*));

std::string get_any_exception_string();
bool        is_exception_active();

struct exception_info {
    void* pointer;
    const std::type_info* type;
    
    exception_info();
};

typedef std::vector<void*> stacktrace_raw;
typedef std::vector<std::string> stacktrace_symbols;

/// Get information about current active exception.
/// @return NULL if no exception is active
bool        get_exception_info(exception_info& foo);
stacktrace_raw get_current_exception_stacktrace();
stacktrace_symbols get_stacktrace_symbols(stacktrace_raw const& stack);


/// Get string describing exception information.
std::string get_any_exception_string(exception_info const& einfo);

/// Call a function object and convert all exceptions to execution_exception
template <typename F>
void monitored_call(F functor);

/// Call a function object and convert all exceptions to execution_exception
template <typename T>
void monitored_call(T& object, void (T::*member)(void));

//
// implementation
//
template <typename T, typename P>
void register_exception_type(std::string (*converter)(P const*))
{
	any_to_string_t any_converer = reinterpret_cast<any_to_string_t>(converter);
	
	register_exception_type(typeid(T), any_converer);
}

template <typename F>
void monitored_call(F functor)
{
	try {
		functor();		
	} catch( ... ) {
		throw execution_exception(get_any_exception_string());
	}
}

template <typename T>
void monitored_call(T& object, void (T::*member)(void))
{
	try {
		(object.*member)();
	} catch( ... ) {
		throw execution_exception(get_any_exception_string());
	}
}


#endif // get_exception_h_included

