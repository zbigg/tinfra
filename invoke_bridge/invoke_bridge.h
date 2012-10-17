#ifndef tinfra_invoke_bridge_h_included
#define tinfra_invoke_bridge_h_included

#include <tinfra/lex.h>
#include <tinfra/fmt.h>
#include <tinfra/typeinfo.h>

#include <vector>
#include <map>
#include <string>
#include <typeinfo>
#include <stdexcept>

#include <iostream>

namespace tinfra {
    
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
        std::cout << "    called " << __PRETTY_FUNCTION__ << "\n";
        
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
struct arg_getter<T const&> {
    static T get(invoke_bridge_context const& ctx, invoke_bridge_context_int& int_ctx) {
        std::cout << "    called " << __PRETTY_FUNCTION__ << "\n";
        
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
        std::cout << "    called " << __PRETTY_FUNCTION__ << "\n";
        
        std::string interface_name = typeid(T).name();
        typename std::map<std::string, void*>::const_iterator iresult = ctx.context.find(interface_name);
        if( iresult == ctx.context.end() ) {
           
            throw std::logic_error(tinfra::tsprintf("unable to find interface '%s'", tinfra::type_name<T>()));
        }
        return *reinterpret_cast<T*>(iresult->second);
    }
};

// TBD
// a simple note, this is the same functionality as in 
// ../reflect/reflect.h:
//    template method_invoker<ReturnType>::invoke<Object,...> which
//  invokes methods but reads from vector<any<
//  idea would be to generalize this code and abstract-out
//  somethins like:
//     * return_value_consumer
//        invoke_bridge: serializer
//        reflect, copy to any
//     * argument_provider
//        invoke_bridge: here invoke_bridge_context
//        reflect: from vector<any>

template <typename R, typename T1>
R invoke_bridge_call(invoke_bridge_context const& ctx, R function(T1))
{
    invoke_bridge_context_int int_ctx;
    int_ctx.consumed_args = 0;
    std::cout << "called " << __PRETTY_FUNCTION__ << "\n";
    T1 arg1 = arg_getter<T1>::get(ctx, int_ctx);
    
    return function(arg1);
}

template <typename R, typename T1, typename T2>
R invoke_bridge_call(invoke_bridge_context const& ctx, R function(T1,T2))
{
    invoke_bridge_context_int int_ctx;
    int_ctx.consumed_args = 0;
    std::cout << "called " << __PRETTY_FUNCTION__ << "\n";
    T1 arg1 = arg_getter<T1>::get(ctx, int_ctx);
    T2 arg2 = arg_getter<T2>::get(ctx, int_ctx);
    
    return function(arg1, arg2); 
}

} // end namespace tinfra

#endif // tinfra_invoke_bridge_h_included

