//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//


#include <stdexcept>

#include "tinfra/fmt.h"

#include "scope.h"

namespace tinfra {

scope::~scope()
{
    clear();
}
void* scope::get_or_create(mapping_key const& key)
{
    mapping_t::const_iterator ie = map.find(key);
    if( ie == map.end() ) {
        return create(key);
    } else {
        // TODO: check for const
        return ie->second.ptr;
    }
}


void* scope::get_no_create(mapping_key const& key) const 
{
    mapping_t::const_iterator ie = map.find(key);
    if( ie == map.end() ) {
        // TODO: this should be fatal application error        
        throw std::logic_error(tinfra::fmt("%s (%s) not available at this scope") % demangle_typeinfo_name(*key.type) % key.key);
    } else {
        // TODO: check for const
        return ie->second.ptr;
    }
}

void  scope::put(mapping_key const& key, void* v)
{
    mapping_value e;
    e.own = false;
    e.is_const = false;
    e.ptr = v;
    map[key] = e;
}

/*
void  scope::put(mapping_key const& key, void const* v)
{
    mapping_value e;
    e.own = false;
    e.is_const = true;
    e.ptr = const_cast<void*>(v);
    map[key] = e;
}
*/

factory* scope::get_factory(type_t t) const
{
    factory_map_t::const_iterator ie = factories.find(t);
    if( ie == factories.end()) ) {
        return 0;
    } else {
        return ie->second;
    }
}
void* scope::create(mapping_key const& key)
{
    factory* f = get_factory(t);
    if( f == 0 ) {
        throw std::logic_error(tinfra::fmt("%s (%s) not available at this scope - create not supported") 
                                            % demangle_typeinfo_name(*key.type) 
                                            % key.key);
    }
    
    // TODO: make following part exception safe
    void* v = f->create(this);
    mapping_value e;
    e.own = true;
    e.is_const = false;
    e.ptr = v;
    map[key] = e;
    
    return p;
}
 
void  scope::clear()
{
    for( mapping_t::iterator ie = map.begin(); ie != map.end(); ++i ) {
        mapping_value& v = ie->second;
        if( v.own ) {
            factory* f = get_factory(ie->first.type);
            if( f == 0 ) {
                // TODO: abort, it's internal error
                abort();
            }
            if( v == 0 ) {
                abort();
            }
            f->destroy(v->ptr);
            v->ptr = 0;
        }
    }
}
void  resolving_failure(mapping_key const&)
{
    throw std::logic_error(tinfra::fmt("%s (%s) not available at this scope - create not supported") 
                                        % demangle_typeinfo_name(*key.type) 
                                        % key.key);
}
} // end namespace tinfra


// HHHHHHHHHHHHHHHHHHHH 
#include <iostream>
#include <string>

using namespace std;
using namespace tinfra;

void fun(scope const& c)
{
    cout << "fun: " << c.get<float>() << " " << c.get<string>() << endl;
}

int main()
{
    scope s;
    float n = 3.14;
    string name("foo");
    s.put<float>(n);
    s.put<string>(name);
    
    fun(s);
}
