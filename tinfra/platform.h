#ifndef __tinfra__platform_h__
#define __tinfra__platform_h__

#ifdef _MSC_VER
#pragma warning( disable: 4512) // assignment operator could not be generated
#pragma warning( disable: 4312) // 
#pragma warning( disable: 4127) // conditional expression is constant

#undef HAVE_SYS_TIME_H
#undef HAVE_OPENDIR

#define HAVE_TIME_H 
#define HAVE_IO_H
#define HAVE_FINDFIRST


#else
#include "tinfra/config.h"
#endif

#include <ios>
#include <cstddef>

namespace tinfra {
	using std::size_t;
	using std::streamsize;
	using ::intptr_t;
}

#endif
