#ifndef tinfra_reflect_h_included
#define tinfra_reflect_h_included

#include <tinfra/any.h> // for tinfra::any
#include <tinfra/assert.h> // for TINFRA_ASSERT

#include <vector>       // for std::vector
#include <string>       // for std::string
#include <typeinfo>     // for std::type_info

namespace tinfra {
namespace reflect {

using std::vector;
using std::string;
using tinfra::any;

struct type_info;
struct type_info;

enum type_info_type {
    TIT_NORMAL           = 1,
    TIT_POINTER          = 2,
    TIT_ARRAY            = 3,
    TIT_REFERENCE        = 4,
    TIT_RVALUE_REFERENCE = 5
};

enum  type_info_modifier {
    TIM_CONST            = 1,
    TIM_VOLATILE         = 2,
    TIM_VIRTUAL          = 4,
    TIM_RESTRICT         = 8
};
    
struct method_info {
    virtual ~method_info() {}
    
    // inquiry
    virtual string              get_name() = 0;
    virtual type_info*         get_return_type() = 0;
    virtual vector<type_info*> get_parameter_types() = 0;
    
    // use
    virtual any          invoke(any const& obj, vector<any>& args) = 0;
};

struct type_info {
    virtual ~type_info();
  
    virtual std::string    name() const = 0;
    
    virtual std::type_info const& get_std_type_info() const = 0;
    
    virtual type_info_type type() const = 0;
    virtual int            modifiers() const = 0;
    
    virtual bool           is(type_info_modifier modifier) const = 0;
        
    // valid in case T
    virtual type_info*    target_type() const = 0;
    
    // retuens non empty is has known methods        
    virtual vector<method_info*>  get_methods() const = 0;
    
    // returns non empty is has known constructors
    // constructor will have signature T fun(args...)
    virtual vector<method_info*>  get_contructors() const = 0;
    
    // capabilities
    virtual bool                  can_copy() const = 0;
    virtual bool                  can_create() const = 0;
    
    // use
    virtual any                   copy(any const&) const = 0;
    virtual any                   create() const = 0;
};

class rtti_system {
    virtual type_info* for_std_type_info(std::type_info const& stdi) = 0;
    virtual type_info* for_name(string const& name) = 0;
    
    // char* -> R.derive(POINTER, R.type_for_name("char")
    // const std::string& -> R.derive(REFERENCE, R.modify(TIM_CONST, R.for_name("std::string")))
    // 
    virtual type_info* derive(type_info_type deriv_type, type_info* target) = 0;
    virtual type_info* modify(int mod, type_info* target) = 0;
};

//
// implementation detail
//

typedef void (*method_invoker_fun)(any, any, any&, vector<any>& args);

class default_method_info: public tinfra::reflect::method_info {    
public:
    default_method_info(std::string name, 
                      type_info* return_type,
                      vector<type_info*> parameter_types,
                      method_invoker_fun invoker,
                      any target_method_ptr);

    template <typename ObjectClass, typename ReturnType>
    static default_method_info* create(const std::string& name, ReturnType (ObjectClass::*fun)());

    template <typename ObjectClass, typename ReturnType, typename T1>
    static default_method_info* create(const std::string& name, ReturnType (ObjectClass::*fun)(T1));

    // inquiry
    string              get_name();
    type_info*         get_return_type();    
    vector<type_info*> get_parameter_types();
    
    // use
    any invoke(any const&, vector<any>& args);
private:
    std::string         name;
    
    type_info*         return_type;
    vector<type_info*> parameter_types;
    
    method_invoker_fun  invoker; // trampoline function
    any                 target_method_ptr; // method to be called

};

template <typename T>
struct remove_ref {
    typedef T type;
};

template <typename T>
struct remove_ref<T&> {
    typedef T type;
};

template <typename T>
struct remove_const {
    typedef T type;
};

template <typename T>
struct remove_const<T const> {
    typedef T type;
};

template <typename ReturnType>
struct method_invoker {
        
    template <typename ObjectClass>
    static void invoke(any obj, any fun, any& ret, vector<any>& args)
    {
        typedef ReturnType (ObjectClass::*MethodPtrType)();
        
        TINFRA_ASSERT ( args.size() == 0 );
        // TBD, it should be typesafe downcast
        ObjectClass& objr = * (reinterpret_cast<ObjectClass*>(obj.get_raw()));
        MethodPtrType method = fun.get<MethodPtrType>();
        ReturnType r = (objr.*method)();
        ret = any::from_copy(r);
    }
    
    template <typename ObjectClass, typename T1>
    static void invoke(any obj, any fun, any& ret, vector<any>& args)
    {
        typedef ReturnType (ObjectClass::*MethodPtrType)(T1);
        
        // TBD, it should be typesafe downcast
        ObjectClass& objr = * (reinterpret_cast<ObjectClass*>(obj.get_raw()));
        
        
        TINFRA_ASSERT ( args.size() == 1 );
        T1& v1 = args[0].get<typename remove_ref<T1>::type>();
        
        MethodPtrType method = fun.get<MethodPtrType>();
        ReturnType r = (objr.*method)(v1);
        ret = any::from_copy(r);
    }
};

template <>
struct method_invoker<void> {
        
    template <typename ObjectClass>
    static void invoke(any obj, any fun, any&, vector<any>& args)
    {
        typedef void (ObjectClass::*MethodPtrType)();
        
        TINFRA_ASSERT ( args.size() == 0 );
        // TBD, it should be typesafe downcast
        ObjectClass& objr = * (reinterpret_cast<ObjectClass*>(obj.get_raw()));
        MethodPtrType method = fun.get<MethodPtrType>();
        (objr.*method)();
    }
    
    template <typename ObjectClass, typename T1>
    static void invoke(any obj, any fun, any&, vector<any>& args)
    {
        typedef void (ObjectClass::*MethodPtrType)(T1);
        
        // TBD, it should be typesafe downcast
        ObjectClass& objr = * (reinterpret_cast<ObjectClass*>(obj.get_raw()));
                
        TINFRA_ASSERT ( args.size() == 1 );
        T1& v1 = args[0].get<typename remove_ref<T1>::type>();
        
        MethodPtrType method = fun.get<MethodPtrType>();
        (objr.*method)(v1);
    }
};

template <typename ObjectClass, typename ReturnType>
default_method_info* default_method_info::create(const std::string& name, ReturnType (ObjectClass::*fun)())
{
    method_invoker_fun mif = &method_invoker<ReturnType>::template invoke<ObjectClass>;
    type_info* return_type_ci = 0;
    vector<type_info*> parameters_civ;
    return new default_method_info(
        name,
        return_type_ci,
        parameters_civ,
        mif,
        any::from_copy(fun));
}

template <typename ObjectClass, typename ReturnType, typename T1>
default_method_info* default_method_info::create(const std::string& name, ReturnType (ObjectClass::*fun)(T1))
{
    method_invoker_fun mif = &method_invoker<ReturnType>::template invoke<ObjectClass, T1>;
    type_info* return_type_ci = 0;
    vector<type_info*> parameters_civ;
    return new default_method_info(
        name,
        return_type_ci,
        parameters_civ,
        mif,
        any::from_copy(fun));
}

} } // end namespace tinfra::reflect

#endif // include guard
