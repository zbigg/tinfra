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

struct invoke_bridge_context {
    std::vector<std::string> raw_args;
    std::map<std::string, void*> context;    
};

struct invoke_bridge_context_int {
    int consumed_args;
};


template <typename T>
struct arg_getter {
    static T get(invoke_bridge_context const& ctx, invoke_bridge_context_int& int_ctx) {
        std::cout << "called " << __PRETTY_FUNCTION__ << "\n";
        
        const int arg_number = int_ctx.consumed_args; 
        if( arg_number >= ctx.raw_args.size() ) {
            throw std::logic_error("not enough arguments");
        }
        std::string const& text_arg = ctx.raw_args[arg_number];
        const T result = tinfra::from_string<T>(text_arg);
        
        int_ctx.consumed_args++;
        
        return result;
    }
};

template <typename T>
struct arg_getter<T&> {    
    static T& get(invoke_bridge_context const& ctx, invoke_bridge_context_int&) {
        std::cout << "called " << __PRETTY_FUNCTION__ << "\n";
        
        std::string interface_name = typeid(T).name();
        typename std::map<std::string, void*>::const_iterator iresult = ctx.context.find(interface_name);
        if( iresult == ctx.context.end() ) {
           
            throw std::logic_error(tinfra::tsprintf("unable to find interface '%s'", tinfra::type_name<T>()));
        }
        return *reinterpret_cast<T*>(iresult->second);
    }
};

#define REGISTER_GLOBAL(some_function)

using std::string;
using std::vector;

template <typename R, typename T1>
void invoke_bridge_call(invoke_bridge_context const& ctx, R function(T1))
{
    invoke_bridge_context_int int_ctx;
    int_ctx.consumed_args = 0;
    std::cout << "called " << __PRETTY_FUNCTION__ << "\n";
    T1 arg1 = arg_getter<T1>::get(ctx, int_ctx);
    
    assert(arg1 == 128);
}

template <typename R, typename T1, typename T2>
void invoke_bridge_call(invoke_bridge_context const& ctx, R function(T1,T2))
{
    invoke_bridge_context_int int_ctx;
    int_ctx.consumed_args = 0;
    std::cout << "called " << __PRETTY_FUNCTION__ << "\n";
    T1 arg1 = arg_getter<T1>::get(ctx, int_ctx);
    T2 arg2 = arg_getter<T2>::get(ctx, int_ctx);
    assert(&arg1 == &std::cout);
    assert(arg2 == 128);    
}
//
//
//
string some_function(int a)
{
    char buf[100];
    sprintf(buf, "0x%08x", a);
    return string(buf);
}

void some_function_out(std::ostream& x, int a)
{
    char buf[100];
    sprintf(buf, "0x%08x", a);
    x << buf << "\n";    
}

REGISTER_GLOBAL(some_function);

void test_base()
{
    invoke_bridge_context ctx;
    ctx.raw_args.push_back("128");
    invoke_bridge_call(ctx, &some_function);
}

void test_base_out()
{
    invoke_bridge_context ctx;
    ctx.raw_args.push_back("128");
    
    ctx.context[typeid(std::ostream).name()] = &std::cout;
    ctx.context[typeid(std::istream).name()] = &std::cin;
    
    invoke_bridge_call(ctx, &some_function_out);
}

int main(int argc, char** argv)
{
    test_base();
    test_base_out();
    
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
