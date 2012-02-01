#include "mo_interface.h"

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
    TINFRA_BEAN_METHOD(basic_scm, get_manifest);
    TINFRA_BEAN_METHOD(basic_scm, get_revision_info);
    TINFRA_BEAN_METHOD(basic_scm, get_head_revision_info);
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
    TINFRA_BEAN_METHOD(lock_manager, login);
    TINFRA_BEAN_METHOD(lock_manager, get_status);
    TINFRA_BEAN_METHOD(lock_manager, create_lock);
    TINFRA_BEAN_METHOD(lock_manager, remove_lock);
}


struct method_visitor {
    
    template <typename CLS, typename R>
    void method(const char* name, R (CLS::*fun)()) {
        using tinfra::type_name;
        std::cout << type_name<R>() << " " << name << "()\n";
    }
    
    template <typename CLS, typename R, typename T>
    void method(const char* name, R (CLS::*fun)(T)) {
        using tinfra::type_name;
        std::cout << type_name<R>() << " " << name << "(" << type_name<T>() << ")\n";
    }
    
    template <typename CLS, typename R, typename T1, typename T2>
    void method(const char* name, R (CLS::*fun)(T1, T2)) {
        using tinfra::type_name;
        std::cout << type_name<R>() << " " << name << "(" << type_name<T1>() << ", " << type_name<T2>() << ")\n";
    }
};

int main()
{
    method_visitor m;
    tinfra::visit_interface<lock_manager>(m);
    
}
