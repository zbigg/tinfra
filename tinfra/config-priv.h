#ifndef tinfra_int_config_h_included
#define tinfra_int_config_h_included

#ifdef _MSC_VER
#pragma warning( disable: 4512) // assignment operator could not be generated
#pragma warning( disable: 4511) // copy constructor could not be generated
#pragma warning( disable: 4312) // 
#pragma warning( disable: 4127) // conditional expression is constant

// some strange warning disablers for recent visuals
// see http://msdn.microsoft.com/en-us/library/aa985965(VS.80).aspx

#define _CRT_SECURE_NO_WARNINGS  1 // yeah, good idea. warn about correct C++ usages
#define _CRT_NONSTDC_NO_WARNINGS 1 // because of someone paranoia
#define _SCL_SECURE_NO_WARNINGS  1 // this one is extremly stupid 

#undef HAVE_SYS_TIME_H
#undef HAVE_OPENDIR

#define HAVE_TIME_H
#define HAVE_IO_H
#define HAVE_FINDFIRST

#else
#include "tinfra/config-priv-autoconf.h"
#endif

#endif // tinfra_int_config_h_included

