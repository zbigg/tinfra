#include "manual_rtti_system.h"


namespace tinfra {
namespace reflect {

//
// tinfra::reflect::manual_type_info
//


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
    // delete registered methods
    for( vector<method_info*>::const_iterator i = this->methods.begin(); i != this->methods.end(); ++i ) {
        delete *i;
    }
}

//
// tinfra::reflect::manual_rtti_system
//

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
        if( i != this->types_by_derivation.end() ) {
            std::cerr << "do_ensure_registered, found by type&target: " << i->second->name() << "\n";
            return i->second;
        }
    }
    if( mod != 0 && target != 0 ) {
        modified_type_key key(mod, target);
        modified_type_key_t::const_iterator i = this->types_by_modifier.find(key);
        if( i != this->types_by_modifier.end() ) {
            std::cerr << "do_ensure_registered, found by mod&target: " << i->second->name() << "\n";
            return i->second;
        }
    }
    if( target == 0 && stdti != 0 ) {
        result = this->for_std_type_info(*stdti);
        if( result ) {
            std::cerr << "do_ensure_registered, found by stdtid: " << result->name() << "\n";
            return result;
        }
    } 
    return do_register_type(name, tit, mod, target, stdti);
}

type_info* manual_rtti_system::do_register_type(std::string const& name, type_info_type tit, int mod, type_info* target, const std::type_info* stdti)
{
    std::string the_name(name);
    if( the_name.empty() )
        the_name = apply_modifiers(tit,mod, target);
    
    std::cerr << "rtti::registering name:" << the_name << " (stdtid " << stdti->name() << ")\n";
    manual_type_info* mti = new manual_type_info(the_name, tit, mod, target, stdti);    
    this->types_by_name[the_name] = mti;
    if( target == 0 ) {
        this->types_by_std_type_info[stdti] = mti;
    }
    
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

} } // end namespace tinfra::reflct

