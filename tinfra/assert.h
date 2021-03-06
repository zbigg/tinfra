//
// Copyright (c) 2011, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_assert_h_included
#define tinfra_assert_h_included

#include "fmt.h"
#include "tinfra/platform.h" // for TINFRA_UNLIKELY
#include "tinfra/trace.h"  // for tinfra::source_location

namespace tinfra { 

/// Report assertion failure.
///
/// Reports a runtime assertion failure described by message and source code 
/// location.
///
/// Default behaviour, delivered by tinfra library is dependable on various
/// parameters
/// - when TINFRA_STRICT_ASSERT environment variable is set, then "reasonable"
///   diagnostics is reported to application and abort() is called
/// - otherwise, std::logic error is thrown, high, process level 
///   cleanup handler can be called
///
/// Reasonable diagnostic information is meant as:
/// - when TINFRA_INTERESTING is used, then current thread interesting state is 
///   is reported
/// - if platform supports, then stacktrace is reporte, posibly with
///   function/symbol names 
///
/// NOTE, TINFRA_INTERESTING, is not yet described but it's meant as thread local
/// stack of important input/state varaiables/actual argumetns
/// that are dumped into log on request, possibly only in case of errors
/// 
/// Called by TINFRA_ASSERT, TINFRA_STATIC_AUTO_ASSERT upon failure.

void report_assertion_failure(tinfra::source_location const& location, const char* message);

/// same as C++ assert, but calls report_assertion_failure.
#define TINFRA_ASSERT(expression) \
    TINFRA_ASSERT_IMPL(expression)

    
#define TINFRA_STATIC_ASSERT(expression) TINFRA_STATIC_ASSERT_IMPL(expression)
    
/// Define static assertion, executed during static initialization
///
/// Effectively caslls assert(expression) during static initialization
/// of program.
///
/// Example:
///    TINFRA_STATIC_AUTO_ASSERT( std::string(basename("foo/bar") == "bar"))
///
/// Will assert when basename doesn't return string "bar".
///
/// Warning, overusage of these statements may result in 
///  - bloated executable size
///  - unexpected behaviour when for example, some library fails during static
///    initialiazation phase
///
/// It's recommended to use as quick replacement for unittests at prototyping
/// stage.
/// Not recommended to use in libraries.

#define TINFRA_STATIC_AUTO_ASSERT(expression)     \
        TINFRA_STATIC_AUTO_ASSERT_IMPL(expression)

//
// implementation
//

#define TINFRA_ASSERT_IMPL(expression) \
    do { if( TINFRA_UNLIKELY (!(expression) ) ) {    \
        tinfra::assert_failed(TINFRA_SOURCE_LOCATION(), #expression); \
        } } while( 0 )

extern void assert_failed(tinfra::source_location const& location, const char* expression);

#define TINFRA_STATIC_AUTO_ASSERT_IMPL(expression)     \
    static struct                                 \
    tinfra_static_auto_asserter##__LINE__ {       \
        tinfra_static_auto_assert() {             \
            TINFRA_ASSERT( expression );          \
        } }                                       \
        tinfra_static_auto_asserer_inst##__LINE__ 

//template<int  x> struct static_assert_p        {};

#if defined(HAVE_STATIC_ASSERT) || defined(TINFRA_CXX11)
// c++11 static_assert
#define TINFRA_STATIC_ASSERT_IMPL(EXPR) \
        static_assert(EXPR, #EXPR)
#else

// static_assert inspired by stackoverflow & boost
//  http://stackoverflow.com/questions/987684/does-gcc-have-a-built-in-compile-time-assert
template<bool x> struct static_assert_f;
template<      > struct static_assert_f <true> {};

#define TINFRA_STATIC_ASSERT_IMPL(EXPR)   \
        enum { dummy = sizeof(tinfra::static_assert_f<((EXPR) ? true : false)>) }

    
#endif

}  // end namespace tinfra

#endif // tinfra_assert_h_included

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

