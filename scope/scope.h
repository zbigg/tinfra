//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_scope_h__
#define __tinfra_scope_h__

#include <map>
#include <typeinfo>

namespace tinfra {

class scope {
public:    
    class factory {
    public:
        virtual void* create(scope&) = 0;
        virtual void  destroy(void*) = 0;
    };

    virtual ~scope();

    typedef int key_type;
    
    template <typename T>
    T& get(int key = -1) {
        mapping_key ik(get_type<T>(), key);
        return * (reinterpret_cast<T*>(get_or_create(ik) ));
    }
    
    template <typename T>
    T& get(int key = -1) const {
        mapping_key ik(get_type<T>(), key);
        return * (reinterpret_cast<T*>( get_no_create(ik) ));
    }

    template <typename T>
    scope& put(T& v, int key=-1) {
        put(mapping_key(get_type<T>(), key), &v);
        return *this;
    }
    
    //template <typename T>
    //scope& put(T const& v, int key=-1) {
    //    put(mapping_key(get_type<T>(), key), &v);
    //    return *this;
    //}
    
    template <typename T>
    scope& factory(factory* f) {
        factories[get_type<T>()] = f;
        return *this;
    }
protected:
    typedef std::type_info const* type_t;

    struct mapping_key {
        type_t   type;
        key_type key;
        
        mapping_key() {}
        mapping_key(type_t t, key_type k): type(t), key(k) {}

        bool operator <(mapping_key const& other) const {
            return type->before(*other.type) || key < other.key;
        }
    };
    struct mapping_value {        
        bool     own;
        bool     is_const;
        void*    ptr;
    };
    
    
    typedef std::map<mapping_key, mapping_value> mapping_t;
    typedef std::map<type_t, factory*>           factory_map_t;
    
    mapping_t map;
    factory_map_t factories;
    
    void* get_or_create(mapping_key const&);    
    void* get_no_create(mapping_key const&) const;
    
    void  put(mapping_key const&, void* v);
    //void  put(mapping_key const&, void const* v);
        
    void* create(mapping_key const&);
    void  clear();
    
    factory* get_factory(type_t) const;
    
    virtual void  resolving_failure(mapping_key const&);
    template <typename T>
    static inline type_t get_type() {
        return & typeid(T);
    }
};


} // end namespace tinfra
 
#endif // __tinfra_scope_h__
