#ifndef tinfra_reflect_manual_rtti_system_included
#define tinfra_reflect_manual_rtti_system_included

#include "reflect.h"

#include <map>
#include "tinfra/typeinfo.h"
#include <iostream> // TBD remove

namespace tinfra {
namespace reflect {
        
class manual_rtti_system: public rtti_system {
    
    typedef std::pair<type_info_type, type_info*> derived_type_key;
    typedef std::pair<int, type_info*> modified_type_key;
    
    typedef std::map<derived_type_key, type_info*> derived_type_map_t;
    typedef std::map<modified_type_key, type_info*> modified_type_key_t;
    
    typedef std::map<std::string, type_info*> name_type_map_t;
    typedef std::map<const std::type_info*, type_info*> stdti_map_t;
    
    derived_type_map_t  types_by_derivation;
    modified_type_key_t types_by_modifier;
    name_type_map_t     types_by_name;
    stdti_map_t         types_by_std_type_info;
public:
    manual_rtti_system();
    
    std::vector<type_info*> get_all();
    
    template <typename T>
    void ensure_basics_registered(std::string const& name);
    
    template <typename T>
    type_info* ensure_registered(std::string const& name);
               
    template <typename T>
    type_info* ensure_registered(std::string const& name, std::vector<method_info*> methods);
    
    type_info* ensure_registered_detail(std::string const& name, type_info_type tit, int mod, type_info* target, const std::type_info& stdti)
    {
        return do_ensure_registered(name, tit, mod, target, &stdti);
    }
    
    type_info* for_std_type_info(std::type_info const& stdi);
    type_info* for_name(string const& name);
    
    type_info* derive(type_info_type deriv_type, type_info* target);    
    type_info* modify(int mod, type_info* target);
protected:
    type_info* do_ensure_registered(std::string const& name, type_info_type tit, int mod, type_info* target, const std::type_info* stdti);
    type_info* do_register_type(std::string const& name, type_info_type tit, int mod, type_info* target, const std::type_info* stdti);
};

//
// implementation detail
// 

struct manual_type_info: public type_info {
    std::string     n;
    
    type_info_type  ti;
    int             mod;
    type_info*      target;
    const std::type_info* std_type_info;
    any             (*copy_fun)(any const&);
    any             (*create_fun)();
    
    vector<method_info*> methods;

    manual_type_info(std::string const& name, type_info_type ti, int mod, type_info* target, std::type_info const* stdti);
    ~manual_type_info();    
    
    std::string name() const
    {
        return n;
    }
    virtual std::type_info const& get_std_type_info() const
    {
        return *this->std_type_info;
    }
    
    virtual type_info_type type() const {
        return this->ti;
    };
    
    virtual int            modifiers() const 
    {
        return this->mod;
    }
    
    virtual bool           is(type_info_modifier modifier) const
    {
        return (this->mod & (int)modifier ) == (int)modifier;
    }
    
        
    // valid in case T
    virtual type_info*    target_type() const
    {
        return this->target;
    }
    
    // retuens non empty is has known methods        
    virtual vector<method_info*>  get_methods() const 
    {
        return this->methods;
    }
    
    // returns non empty is has known constructors
    // constructor will have signature T fun(args...)
    virtual vector<method_info*>  get_contructors() const 
    {
        vector<method_info*> result;
        return result;
    }
    
    // capabilities
    virtual bool                  can_copy() const   { return copy_fun != 0; }
    virtual bool                  can_create() const { return create_fun != 0; }
    
    virtual any                   copy(any const& original) const
    {
        TINFRA_ASSERT(this->copy_fun != 0);
        return this->copy_fun(original);
    }
    
    virtual any                   create() const 
    {
        TINFRA_ASSERT(this->create_fun != 0);
        return this->create_fun();
    }
};

template <typename T>
struct registration_harness {
    manual_rtti_system& R;
    type_info* ensure_registered(std::string const& name)
    {
        std::cerr << "ensure_registered<T>, T=" << tinfra::type_name<T>() << "\n";
        return R.ensure_registered_detail(name, TIT_NORMAL, 0, 0, typeid(T));
    }
};


template <typename T>
struct registration_harness<T&> {
    manual_rtti_system& R;
    
    type_info* ensure_registered(std::string const& name)
    {
        std::cerr << "ensure_registered<T&>, T=" << tinfra::type_name<T>() << "\n";
        
        registration_harness<T> rh = { R };
        type_info* target = rh.ensure_registered("");
        return R.ensure_registered_detail("", TIT_REFERENCE, 0, target, typeid(T&));
    }
};

template <typename T>
struct registration_harness<T*> {
    manual_rtti_system& R;
    
    type_info* ensure_registered(std::string const& name)
    {
        std::cerr << "ensure_registered<T*>, T=" << tinfra::type_name<T>() << "\n";
        registration_harness<T> rh = { R };
        type_info* target = rh.ensure_registered(""); 
        return R.ensure_registered_detail("", TIT_POINTER, 0, target, typeid(T*));
    }
};

template <typename T>
struct registration_harness<T const> {
    manual_rtti_system& R;
    
    type_info* ensure_registered(std::string const& name)
    {
        std::cerr << "ensure_registered<T const>, T=" << tinfra::type_name<T>() << "\n";
        registration_harness<T> rh = { R };
        type_info* target = rh.ensure_registered("");
        return R.ensure_registered_detail("", target->type(), TIM_CONST, target, typeid(T const));
    }
};


template <typename T>
void manual_rtti_system::ensure_basics_registered(std::string const& name)
{
    this->ensure_registered<T>(name);
    this->ensure_registered<T const>("");
    this->ensure_registered<T*>("");
    this->ensure_registered<T const*>("");
    this->ensure_registered<T&>("");
    this->ensure_registered<T const&>("");
}
template <typename T>
type_info* manual_rtti_system::ensure_registered(std::string const& name)
{    
    std::cerr << "rtti: ensure_registered, T=" << tinfra::type_name<T>() << "\n";
    registration_harness<T> rh = { *this } ;
    type_info* r = rh.ensure_registered(name);
    std::cerr << "...\n";
    return r;
}

std::string apply_modifiers(type_info_type tit, int mod, type_info* target);


} } // end namespace tinfra::reflct

#endif // tinfra_reflect_manual_rtti_system_included
