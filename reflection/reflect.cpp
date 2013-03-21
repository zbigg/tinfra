#include "reflect.h"

#include <boost/function.hpp>

namespace tinfra {
namespace reflect {

//
// type_info
// 
    
type_info::~type_info() {}
    
//
// default_method_info
// 
default_method_info::default_method_info(
                  std::string name, 
                  type_info* return_type,
                  vector<type_info*> parameter_types,
                  method_invoker_fun invoker,
                  any target_method_ptr):
    name(name),
    return_type(return_type),
    parameter_types(parameter_types),
    invoker(invoker),
    target_method_ptr(target_method_ptr)

{
}

string      default_method_info::get_name() {
    return this->name;
}

type_info* default_method_info::get_return_type() { 
    return this->return_type; 
}

vector<type_info*> default_method_info::get_parameter_types() { 
    return this->parameter_types;
}

// use

any default_method_info::invoke(any const& obj, vector<any>& args)
{
    any result = any::from_copy(0);
    this->invoker(obj, this->target_method_ptr, result, args);
    return result;
}

} }
