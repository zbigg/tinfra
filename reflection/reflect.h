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

struct class_info;
    
struct method_info {
    virtual ~method_info() {}
    
    // inquiry
    virtual string              get_name() = 0;
    virtual class_info*         get_return_type() = 0;
    virtual vector<class_info*> get_parameter_types() = 0;
    
    // use
    virtual any          invoke(any const& obj, vector<any>& args) = 0;
};

struct class_info {
    virtual ~class_info() {}
    
    // inquiry
    virtual std::type_info const& get_type_info() = 0;
    virtual vector<method_info*>  get_methods() = 0;
    virtual vector<method_info*>  get_contructors() = 0;
    
    // capabilities
    virtual bool                  can_copy() = 0;
    virtual bool                  can_create() = 0;
    
    // use
    virtual any                   copy(any const&) = 0;
    virtual any                   create() = 0;
};

class rtti_system {
    virtual class_info*      class_for_name(string const& name) = 0;
};

//
// implementation detail
//

typedef void (*method_invoker_fun)(any, any, any&, vector<any>& args);

class default_method_info: public tinfra::reflect::method_info {    
public:
    default_method_info(std::string name, 
                      class_info* return_type,
                      vector<class_info*> parameter_types,
                      method_invoker_fun invoker,
                      any target_method_ptr);

    template <typename ObjectClass, typename ReturnType>
    static default_method_info* create(const std::string& name, ReturnType (ObjectClass::*fun)());

    template <typename ObjectClass, typename ReturnType, typename T1>
    static default_method_info* create(const std::string& name, ReturnType (ObjectClass::*fun)(T1));

    // inquiry
    string              get_name();
    class_info*         get_return_type();    
    vector<class_info*> get_parameter_types();
    
    // use
    any invoke(any const&, vector<any>& args);
private:
    std::string         name;
    
    class_info*         return_type;
    vector<class_info*> parameter_types;
    
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
    class_info* return_type_ci = 0;
    vector<class_info*> parameters_civ;
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
    class_info* return_type_ci = 0;
    vector<class_info*> parameters_civ;
    return new default_method_info(
        name,
        return_type_ci,
        parameters_civ,
        mif,
        any::from_copy(fun));
}

} } // end namespace tinfra::reflect

#endif // include guard
