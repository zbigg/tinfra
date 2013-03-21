#include "mo_interface.h"
#include "manual_rtti_system.h"

#include <tinfra/mo.h>

#include <tinfra/typeinfo.h>

#include <string>
#include <iostream>

struct basic_manifest {
};

struct revision_info {
};

class basic_scm {
        
    virtual basic_manifest get_manifest(std::string const& revid) = 0;
    virtual revision_info  get_revision_info(std::string const& revid) = 0;
    virtual revision_info  get_head_revision_info() = 0;

};

//
// ok, we 
TINFRA_BEAN_MANIFEST(basic_scm) {
    TINFRA_BEAN_METHOD(get_manifest);
    TINFRA_BEAN_METHOD(get_revision_info);
    TINFRA_BEAN_METHOD(get_head_revision_info);
}

struct lock_managet_status {
};

typedef int lock_handle;

class lock_manager {
    
    virtual void login(std::string user, std::string pass) = 0;
    
    virtual lock_managet_status get_status() = 0;
    virtual lock_handle         create_lock(std::string const& name) = 0;
    virtual lock_handle         remove_lock(lock_handle& lh) = 0;
    
};

//
// ok, we
TINFRA_BEAN_MANIFEST(lock_manager) {
    TINFRA_BEAN_METHOD(login);
    TINFRA_BEAN_METHOD(get_status);
    TINFRA_BEAN_METHOD(create_lock);
    TINFRA_BEAN_METHOD(remove_lock);
}


struct method_visitor {
    tinfra::reflect::manual_rtti_system&    rtti;
    std::vector<tinfra::reflect::method_info*> methods;
    
    template <typename CLS, typename R>
    void method(const char* name, R (CLS::*fun)()) {
        using tinfra::type_name;
        std::cout << type_name<R>() << " " << name << "()\n";
        
        tinfra::reflect::method_info* mi = tinfra::reflect::default_method_info::create<CLS>(name, fun);
        methods.push_back(mi);
    }
    
    template <typename CLS, typename R, typename T>
    void method(const char* name, R (CLS::*fun)(T)) {
        using tinfra::type_name;
        std::cout << type_name<R>() << " " << name << "(" << type_name<T>() << ")\n";
        
        tinfra::reflect::method_info* mi = tinfra::reflect::default_method_info::create<CLS>(name, fun);
        methods.push_back(mi);
    }
    
    template <typename CLS, typename R, typename T1, typename T2>
    void method(const char* name, R (CLS::*fun)(T1, T2)) {
        using tinfra::type_name;
        std::cout << type_name<R>() << " " << name << "(" << type_name<T1>() << ", " << type_name<T2>() << ")\n";
        
        //tinfra::reflect::method_info* mi = tinfra::reflect::default_method_info::create<CLS>(name, fun);
        //methods.push_back(mi);
    }
};

template <typename BeanType>
void rtti_register_interface(tinfra::reflect::manual_rtti_system& rtti, std::string const& name)
{
    method_visitor m { rtti, };
    tinfra::visit_interface<BeanType>(m);
    
    tinfra::reflect::type_info* ti = rtti.ensure_registered<BeanType>(name, m.methods);
}

int main()
{
    
    tinfra::reflect::manual_rtti_system rtti;
    rtti_register_interface<lock_manager>(rtti, "lock_manager");
}
