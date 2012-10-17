//
// invoke_bridge
//
// this is realization of following idea
//
// * cli/http/gui univeral interface
//    so i can write
//      int foo(int a,int c)
//    and have automagical http/cli/desktop
//    interface towards this function
//
// * c++ framework to write down easy small web applications
// 
//      real fucking easy mapper from data
//      processing into web interface, i.e
// 
//      vector<string> foo(int id) {
//      }
//      HTTP_MAP(foo, "/foo/{1}");
//
//
// requirements:
//   for given free function R foo(T...)
//   it shall be possible to register it in some global list
//   so other programs can 
//     1) call it with apropriate params and 
//     2) consume result
//     3) query about it's params reqiuirement
//     4) optional: to get metadata about args (name, ranges, domains)
//    
//   there exists at least one "invoke engine" that can
//   collect arguments from CLI, convert them to final types
//   and call correct registered funtion example
//
//     int main(int argc, char** argv)
//         if( std::string(argv[1]) == "global_call" ) {
//              // will collect args from ARGV, and
//              // result will be printed serialized to stdout
//              return global_cli_call(argv[2], vector<string>(argv+3, argv+argc); // more or less
//         }
//         if( std::string(argv[1]) == "global_list" ) {
//              global_cli_list(); return 0
//         }
//
//  example:
//    somefile.cpp:
//      #include "invoke_bridge.h"
//      std::vector<std::string> get_system_users() {...}
//      REGISTER_GLOBAL(get_system_users)
//    
//  issues
//    compilation border i.e i'd like to expose functions
//    that inherently do now know how they will be called
//      
//  what works ?
//    effectively deducing  
//      - value_types (T and T const&)
//      - reference (T&) 
//    and retrieving them from context (invoke_bridge_context)
//    
//

#include "invoke_bridge.h"

#include <vector>
#include <string>
#include <stdio.h>
#include <iostream>
#include <cassert>
#include <typeinfo>
#include <map>
#include <stdexcept>
#include <sstream>

#include <tinfra/fmt.h>
#include <tinfra/lex.h>
#include <tinfra/typeinfo.h>
#include <tinfra/test.h>
#include <tinfra/test_macros.h>

using tinfra::invoke_bridge_context;
using tinfra::invoke_bridge_call;


std::string some_function(int a)
{
    CHECK_EQUAL(128, a);
    char buf[100];
    sprintf(buf, "0x%08x", a);
    return std::string(buf);
}

TEST(invoke_bridge_base)
{
    invoke_bridge_context ctx;
    ctx.raw_args.push_back("128");
    std::string x = invoke_bridge_call(ctx, &some_function);
    CHECK_EQUAL( "0x00000080", x );
}

void some_function_ref(std::ostream& x, int a)
{
    CHECK_EQUAL(255, a);
    char buf[100];
    sprintf(buf, "0x%08x", a);
    x << buf << "\n";    
}

TEST(invoke_bridge_base_ref)
{
    invoke_bridge_context ctx;
    ctx.raw_args.push_back("255");
    std::ostringstream x;
    ctx.context[typeid(std::ostream).name()] = &x;
    
    invoke_bridge_call(ctx, &some_function_ref);
    
    CHECK_EQUAL("0x000000ff\n", x.str());
}

std::string some_function_const_ref(int a, std::string const& s)
{
    CHECK_EQUAL(1, a);
    CHECK_EQUAL("abc", s);
    char buf[100];
    sprintf(buf, "0x%08x %i", a, s.size());
    return std::string(buf);
}

TEST(invoke_bridge_base_const_ref)
{
    invoke_bridge_context ctx;
    ctx.raw_args.push_back("1");
    ctx.raw_args.push_back("abc");
    
    const std::string r = invoke_bridge_call(ctx, &some_function_const_ref);
    
    CHECK_EQUAL("0x00000001 3", r);
}

TEST(invoke_bridge_registries)
{
#if 0    
    assert(argc > 1 );
    if( string(argv[1]) == "global_call" ) {
     // will collect args from ARGV, and
     // result will be printed serialized to stdout
     assert(argc > 2);
     vector<string> args(argv+3, argv+argc);
     return global_cli_call(argv[2], args); // more or less
    }
    if( std::string(argv[1]) == "global_list" ) {
         global_cli_list(); return 0
    }
#endif    
}

int main(int argc, char** argv)
{
    return tinfra::test::test_main(argc, argv);
}


