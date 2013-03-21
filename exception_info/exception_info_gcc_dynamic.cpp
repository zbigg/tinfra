#include "exception_info.h"
#include <dlfcn.h>  	// for ::dlsym, RTL
#include <iostream>
#include <typeinfo>
#include <cstdlib>

//
// basic, dynamic-linked-based overload
//

typedef void (*cxa_throw_type)(void* , void *, void (*) (void *));

static cxa_throw_type orig_cxa_throw = 0;

static cxa_throw_type load_orig_throw_code()
{
	cxa_throw_type cxa_throw_ptr = (cxa_throw_type)( dlsym(RTLD_NEXT, "__cxa_throw") );
	//std::cerr << "cxa_throw_hijack: loaded original cxa_throw " << (void*)cxa_throw_ptr << "\n";
	return cxa_throw_ptr;
}

extern "C" 
void __cxa_throw(void *thrown_exception, void *pvtinfo, void (*dest) (void *) )
{
	std::type_info const* tinfo = reinterpret_cast<std::type_info const*>(pvtinfo);	
	if( orig_cxa_throw == 0 ) {
		orig_cxa_throw = load_orig_throw_code();
	}
	if( orig_cxa_throw == 0 || orig_cxa_throw == &__cxa_throw ) {
		std::cerr << "CXA_THROW: unable to find original g++ throw routine (__cxa_throw), aborting " << tinfo->name()<< "\n";
		abort();
	}
	
	tinfra::exception_info_callback(thrown_exception, tinfo);

	
	orig_cxa_throw(thrown_exception, pvtinfo, dest);
}
