#include "exception_info.h"

//
// static __cxa_throw hook
//
// the assumption is that we link with MODIFIED! version of libstdc++
// this version shall have renamed '__cxa_throw' implementation to '__libstdcxx_cxa_throw'
//
// rename__cxa_throw.sh creates such version of libstdc++, named libstdc++-tinfra-throw.a

extern "C"
void __libstdcxx_cxa_throw(void*, void*, void (*) (void*));

extern "C" 
void __cxa_throw(void *thrown_exception, void *pvtinfo, void (*dest) (void *) )
{
	std::type_info const* tinfo = reinterpret_cast<std::type_info const*>(pvtinfo);

	tinfra::exception_info_callback(thrown_exception, tinfo);

	__libstdcxx_cxa_throw(thrown_exception, pvtinfo, dest);
}
