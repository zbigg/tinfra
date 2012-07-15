#include "reflect.h"

#include "tinfra/test.h"
#include "tinfra/cmd.h"

#include <map>
#include <iostream>

namespace tinfra {
namespace reflect {

type_info::~type_info() {}

struct manual_type_info: public type_info {
    std::string     n;
    
    type_info_type  ti;
    int             mod;
    type_info*      target;
    const std::type_info* std_type_info;
    any             (*copy_fun)(any const&);
    any             (*create_fun)();

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
        vector<method_info*> result;
        return result;
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

manual_type_info::manual_type_info(std::string const& name, type_info_type ti, int mod, type_info* target, std::type_info const* stdti):
    n(name),
    ti(ti),
    mod(mod),
    target(target),
    std_type_info(stdti),
    copy_fun(0),
    create_fun(0)
{
}

manual_type_info::~manual_type_info()
{
}

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

template <typename T>
struct registration_harness {
    manual_rtti_system& R;
    type_info* ensure_registered(std::string const& name)
    {
        return R.ensure_registered_detail(name, TIT_NORMAL, 0, 0, typeid(T));
    }
};

template <typename T>
struct registration_harness<T&> {
    manual_rtti_system& R;
    type_info* ensure_registered(std::string const& name)
    {
        type_info* target = R.for_std_type_info(typeid(T));
        return R.ensure_registered_detail(name, TIT_REFERENCE, 0, target, typeid(T&));
    }
};

template <typename T>
struct registration_harness<T*> {
    manual_rtti_system& R;
    
    type_info* ensure_registered(std::string const& name)
    {
        type_info* target = R.for_std_type_info(typeid(T)); 
        return R.ensure_registered_detail(name, TIT_POINTER, 0, target, typeid(T*));
    }
};

template <typename T>
struct registration_harness<T const> {
    manual_rtti_system& R;
    
    type_info* ensure_registered(std::string const& name)
    {
        type_info* target = R.for_std_type_info(typeid(T)); 
        return R.ensure_registered_detail(name, target->type(), TIM_CONST, target, typeid(T const));
    }
};

std::string apply_modifiers(type_info_type tit, int mod, type_info* target)
{
    // when we derive|modify, we construct name
    // using target type
    std::string the_name;
    TINFRA_ASSERT(target != 0);        
    the_name.append(target->name());
    if( (mod & TIM_CONST) == TIM_CONST ) {
        the_name.append(" const");
    }
    if( (mod & TIM_VOLATILE ) == TIM_VOLATILE ) {
        the_name.append(" volatile");
    }
    switch( tit ) {
    
    case TIT_POINTER:
        the_name.append("*");
        break;
    case TIT_REFERENCE:
        the_name.append("&");
        break;
    case TIT_RVALUE_REFERENCE:
        the_name.append("&&");
        break;
    case TIT_NORMAL:        
    default: 
        break;
    } 
    return the_name;
}

template <typename T>
void manual_rtti_system::ensure_basics_registered(std::string const& name)
{
    this->ensure_registered<T>(name);
    this->ensure_registered<T*>("");
    this->ensure_registered<const T*>("");
    this->ensure_registered<T&>("");
}
template <typename T>
type_info* manual_rtti_system::ensure_registered(std::string const& name)
{    
    registration_harness<T> rh = { *this } ;
    return rh.ensure_registered(name);
}

manual_rtti_system::manual_rtti_system()    
{
    this->ensure_basics_registered<char>("char");
    this->ensure_basics_registered<signed char>("signed char");
    this->ensure_basics_registered<unsigned char>("unsigned char");
    this->ensure_basics_registered<int>("int");
    this->ensure_basics_registered<unsigned int>("unsigned int");
    this->ensure_basics_registered<long>("long");
    this->ensure_basics_registered<unsigned long>("unsigned long");
    this->ensure_registered<void>("void");
    this->ensure_registered<void*>("");
    this->ensure_registered<const void*>("");
}

std::vector<type_info*> manual_rtti_system::get_all()
{
    std::vector<type_info*> result;
    for( name_type_map_t::const_iterator i = this->types_by_name.begin(); i != this->types_by_name.end(); ++i ) {
        result.push_back(i->second);
    }
    return result;
}
type_info* manual_rtti_system::do_ensure_registered(std::string const& name, type_info_type tit, int mod, type_info* target, const std::type_info* stdti)
{
    type_info* result;
    if( tit != TIT_NORMAL && target != 0 ) {
        derived_type_key key(tit, target);
        derived_type_map_t::const_iterator i = this->types_by_derivation.find(key);
        if( i != this->types_by_derivation.end() ) 
            return i->second;
    }
    if( mod != 0 && target != 0 ) {
        modified_type_key key(mod, target);
        modified_type_key_t::const_iterator i = this->types_by_modifier.find(key);
        if( i != this->types_by_modifier.end() ) 
            return i->second;
    }
    if( stdti != 0 ) {
        result = this->for_std_type_info(*stdti);
        if( result )
            return result;
    } 
    return do_register_type(name, tit, mod, target, stdti);
}

type_info* manual_rtti_system::do_register_type(std::string const& name, type_info_type tit, int mod, type_info* target, const std::type_info* stdti)
{
    std::string the_name(name);
    if( the_name.empty() )
        the_name = apply_modifiers(tit,mod, target);
    manual_type_info* mti = new manual_type_info(the_name, tit, mod, target, stdti);    
    this->types_by_name[the_name] = mti;
    this->types_by_std_type_info[stdti] = mti;
    
    if( target != 0 && mod != 0 ) {        
        modified_type_key key(mod, target);
        this->types_by_modifier[key] = mti;
    }
    if( target != 0 && tit != TIT_NORMAL ) 
    {
        derived_type_key key(tit, target);
        this->types_by_derivation[key] = mti;
    }
    
    return mti;
}

type_info* manual_rtti_system::for_std_type_info(std::type_info const& stdti)
{
    stdti_map_t::const_iterator i = this->types_by_std_type_info.find(&stdti);
    if( i == this->types_by_std_type_info.end() )
        return 0;
    return i->second;
}
type_info* manual_rtti_system::for_name(string const& name)
{
    name_type_map_t::const_iterator i = this->types_by_name.find(name);
    if( i == this->types_by_name.end() )
        return 0;
    return i->second;
}

// char* -> R.derive(POINTER, R.type_for_name("char")
// const std::string& -> R.derive(REFERENCE, R.modify(TIM_CONST, R.for_name("std::string")))
// 
type_info* manual_rtti_system::derive(type_info_type deriv_type, type_info* target)
{
    derived_type_key key(deriv_type, target);
    derived_type_map_t::const_iterator i = this->types_by_derivation.find(key);
    if( i == this->types_by_derivation.end() ) {
        return do_ensure_registered("", deriv_type, 0, target, 0);
    } else {
        return i->second;
    }
}

type_info* manual_rtti_system::modify(int mod, type_info* target) 
{
    modified_type_key key(mod, target);
    modified_type_key_t::const_iterator i = this->types_by_modifier.find(key);
    if( i == this->types_by_modifier.end() ) {
        return do_ensure_registered("", target->type(), mod, target, 0);
    } else {
        return i->second;
    }
}

} } // end namespace tinfra::reflect

//
// test application
//

struct lock_managet_status {
    int lock_count;
};

typedef int lock_handle;

struct foo_interface {
    
    virtual lock_managet_status no_arg() = 0;
    virtual lock_handle         one_arg_const_ref(std::string const& name) = 0;
    virtual int                 one_arg_ref(std::string& result) = 0;
    virtual void                no_return(std::string& result) = 0;
    //virtual lock_handle         remove_lock(lock_handle& lh) = 0;
    
};

struct foo_implementation: public foo_interface {
    virtual lock_managet_status no_arg()
    {
        lock_managet_status res = { 666 };
        return res;
    }
    
    virtual lock_handle one_arg_const_ref(std::string const& foo)
    {
        TINFRA_ASSERT(foo == "abcdefghijklmnoprstuvw1registration_registration_harness34567890");
        return 777;
    }
    
    virtual int one_arg_ref(std::string& foo)
    {
        TINFRA_ASSERT(foo == "encepencewktorejrence");
        foo = "bols";
        return 1;
    }
    
    virtual void no_return(std::string& foo)
    {
        TINFRA_ASSERT(foo == "encepencewktorejrence");
        foo = "bols";
    }
};

#define CHECK_TYPES_EQUAL(a, b) \
    CHECK_EQUAL( typeid(a).name(), typeid(b).name() ) 

TEST(remove_const_test)
{
    using tinfra::reflect::remove_const;
    
    //CHECK_TYPES_EQUAL( int, float );
    
    CHECK_TYPES_EQUAL( int, typename remove_const<int>::type );
    CHECK_TYPES_EQUAL( int, remove_const<const int>::type);
    CHECK_TYPES_EQUAL( int, remove_const<int const>::type);
    CHECK_TYPES_EQUAL( int*, remove_const<int* const>::type);
    // and doesn't remove const of target type 
    CHECK_TYPES_EQUAL( const int&, remove_const<const int&>::type);    
}


TEST(remove_ref_test)
{
    tinfra::test::test_run_summary trs;
    
    using tinfra::reflect::remove_ref;
    
    CHECK_TYPES_EQUAL( int, remove_ref<int>::type);
    CHECK_TYPES_EQUAL( int, remove_ref<int&>::type);
    CHECK_TYPES_EQUAL( int&, remove_ref<const int&>::type);
    CHECK_TYPES_EQUAL( int*, remove_ref<int*&>::type);    
}

TEST(manual_rtti_system_basics)
{
    using namespace tinfra::reflect;
    manual_rtti_system R;
    
    type_info* ti = R.for_name("char");
    
    CHECK_EQUAL("char",     ti->name());
    CHECK_EQUAL(TIT_NORMAL, ti->type());
    CHECK_EQUAL(0,          ti->modifiers());
}

TEST(manual_rtti_system_derivation)
{
    using namespace tinfra::reflect;
    manual_rtti_system R;
    
    type_info* rti = R.ensure_registered<char*>("char*");
    type_info* ti  = R.for_name("char*");    
    CHECK_EQUAL(rti, ti);
    
    // check that target is the same as one registered for "char'
    type_info* char_ti  = R.for_name("char");
    CHECK_EQUAL(char_ti, ti->target_type());
    
    // check that later derivation returns same
    type_info* derived_ti  = R.derive(TIT_POINTER, char_ti);
    CHECK_EQUAL(derived_ti, rti);
    
    CHECK_EQUAL("char*",     ti->name());
    CHECK_EQUAL(TIT_POINTER, ti->type());
}

TEST(manual_rtti_check_all)
{
    using namespace tinfra::reflect;
    manual_rtti_system R;
    
    R.ensure_basics_registered<manual_rtti_system>("tinfra::reflect::manual_rtti_system");
    
    
    std::vector<type_info*> all_types = R.get_all();
    int idx = 0;
    for( std::vector<type_info*>::const_iterator i = all_types.begin(); i != all_types.end(); ++i ) {
        type_info* ti = *i;
        
        std::cout << idx++ <<        
             ": name:" << ti->name() <<
             "  type:" << ti->type() <<
             "  mod:" << ti->modifiers() << 
             "  target: " << (ti->target_type() 
                              ? ti->target_type()->name() 
                              : std::string("n/a") ) << 
             "\n";
                                
    }
}

TEST(reflect_test)
{
    using std::vector;
    using tinfra::any;
    using namespace tinfra::reflect;
    using tinfra::reflect::remove_const;


    foo_implementation lm;
    {
        method_info* mi  = default_method_info::create("no_arg", &foo_interface::no_arg);
        vector<any> args;
        any result = mi->invoke(any::by_ref(lm), args);
        
        lock_managet_status ifoo = result.get<lock_managet_status>();
        CHECK_EQUAL(666, ifoo.lock_count);
    }
    {
        method_info* mi = default_method_info::create("one_arg_const_ref", &foo_interface::one_arg_const_ref);
        vector<any> args;
        args.push_back(any::from_copy<std::string>("abcdefghijklmnoprstuvw1registration_registration_harness34567890"));
        any result = mi->invoke(any::by_ref(lm), args);
        
        lock_handle ifoo = result.get<lock_handle>();
        CHECK_EQUAL(777, ifoo);
    }
    
    {
        method_info* mi = default_method_info::create("one_arg_ref", &foo_interface::one_arg_ref);
        vector<any> args;
        std::string foo("encepencewktorejrence");
        args.push_back(any::by_ref(foo));
        any result = mi->invoke(any::by_ref(lm), args);
        
        CHECK_EQUAL("bols", foo);
    }
    
    {
        method_info* mi = default_method_info::create("no_return", &foo_interface::no_return);
        vector<any> args;
        std::string foo("encepencewktorejrence");
        args.push_back(any::by_ref(foo));
        mi->invoke(any::by_ref(lm), args);
        
        CHECK_EQUAL("bols", foo);
    }
}

TINFRA_MAIN(tinfra::test::test_main);

