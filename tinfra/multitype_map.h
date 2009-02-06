//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_multitype_map__
#define __tinfra_multitype_map__

#include <map>
#include <typeinfo>

namespace tinfra {

namespace detail { 
struct dynamic_wrapper_base {
    virtual ~dynamic_wrapper_base() {}
};

template<typename T>
struct dynamic_wrapper: public dynamic_wrapper_base {
    T value;
    virtual ~dynamic_wrapper() {}        
};
}

template <typename K>
class multitype_map {
    typedef K key_type;
    
public:
    ~multitype_map() { clear(); }
    
    /** Returns true if this map contains a mapping for the specified key (and typename). */
    template<typename T>
    bool contains(K const& k) const {
        type_key mkey = get_type_key<T>();
        std::map<K, T> const* m = get_type_map<T>(mkey);
        if( !m ) return false;
        typename std::map<K, T>::const_iterator i = m->find(k);
        return i != m->end();
    }
    
    /** Returns true if this map contains map of given T */
    template<typename T>
    bool contains_type() const {
        std::map<K, T> const* m = get_type_map<T>(get_type_key<T>());
        return m != 0;
    }
    
    /** Returns true if this map contains map of given T */    
    bool contains_type(std::type_info const& type) const {
        wrapper_map_t::const_iterator mapi = wrapper_map.find(&type);
        return mapi != wrapper_map.end();
    }
    
    /**
    * Get value
    *
    * If map is missing value of T with key k then reference to empty value is
    * returned 
    */
    template<typename T>
    T const& get(K const& k) const {
        static T dummy;
        type_key mkey = get_type_key<T>();
        std::map<K, T> const* m = get_type_map<T>(mkey);
        if( !m ) 
            throw std::exception();
        typename std::map<K, T>::const_iterator i = m->find(k);
        if( i == m->end() ) 
            throw std::exception();
        return *i;
    }
    
    /**
    * Get value
    *
    * If map is missing value of T with key k then reference to empty value is
    * returned 
    */
    template<typename T>
    T get(K const& k, T const& default_value) const {
        static T dummy;
        type_key mkey = get_type_key<T>();
        std::map<K, T> const* m = get_type_map<T>(mkey);
        if( !m ) 
            return default_value;
        typename std::map<K, T>::const_iterator i = m->find(k);
        if( i == m->end() ) 
            return default_value;
        return *i;
    }
    
    /***
    * Get value.
    *
    * Value may be modified.
    *
    * If specified value is missing, then reference to newly created value instance
    * is returned 
    */
    template<typename T>
    T & get(K const& k) {
        type_key mkey = get_type_key<T>();
        std::map<K, T>* m = get_type_map<T>(mkey,true);
        if( !m ) 
            throw std::exception();        
        return (*m)[k];
    }
    
    /***
    * Associates value with key.
    *
    * Value may be modified.
    *
    * If specified value is missing, then reference to newly created value instance
    * is returned 
    */
    template <typename T>
    void     put(K const& k, T const& v) {
        type_key mkey = get_type_key<T>();
        std::map<K, T>* m = get_type_map<T>(mkey,true);
        typename std::map<K, T>::value_type p(k,v);
        m->insert(p);
    }
    
    template <typename T>
    typename std::map<K, T>::iterator begin() {        
        return get_type_map<T>(get_type_key<T>(),true)->begin();
    }
    
    template <typename T>
    typename std::map<K, T>::const_iterator begin() const {
        std::map<K, T> const* m = get_type_map<T>(get_type_key<T>());
        if( m ) return m->begin();
        return std::map<K, T>::const_iterator();
    }
    
    template <typename T>
    typename std::map<K, T>::iterator end() {
        return get_type_map<T>(get_type_key<T>(),true)->end();
    }
    
    template <typename T>
    typename std::map<K, T>::const_iterator end() const {
        std::map<K, T> const* m = get_type_map<T>(get_type_key<T>());
        if( m ) return m->end();
        return std::map<K, T>::const_iterator();
    }
    
    /// XXX: should we put it here ?
    // i think no
    //void put(K const& k, const char* str)
    //{
    //    put(k,std::string(str));
    //}
    
    void clear()
    {
        for( wrapper_map_t::iterator i = wrapper_map.begin(); i != wrapper_map.end(); ++i ) {
            detail::dynamic_wrapper_base* x = i->second;
            i->second = 0;
            delete x;
        }
        wrapper_map.clear();
    }
private:
    typedef std::type_info const* type_key;
    template <typename T>
    static inline type_key get_type_key() {
        return &typeid(T);
    }
    
    typedef std::map<type_key, detail::dynamic_wrapper_base*> wrapper_map_t;
    wrapper_map_t wrapper_map;
    
    template<typename T>
    std::map<K, T>* get_type_map(type_key key, bool force_create = false) {
        wrapper_map_t::iterator mapi = wrapper_map.find(key);
        
        detail::dynamic_wrapper<std::map<K,T> >* tb;
        if( mapi == wrapper_map.end() ) {
            tb = new detail::dynamic_wrapper<std::map<K, T> >();
            wrapper_map[key] = tb;
        } else if( force_create ) {
            detail::dynamic_wrapper_base* b = mapi->second;
            tb = dynamic_cast<detail::dynamic_wrapper<std::map<K,T> >* >(b);
            if( !tb ) return 0;

        } else {
            return 0;
        }
        return & tb->value;
    }
    template<typename T>
    std::map<K, T> const* get_type_map(type_key key) const {
        wrapper_map_t::const_iterator mapi = wrapper_map.find(key);
         if( mapi == wrapper_map.end() ) return 0;
         detail::dynamic_wrapper_base* b = mapi->second;
         detail::dynamic_wrapper<std::map<K,T> >* tb = dynamic_cast<detail::dynamic_wrapper<std::map<K,T> > *>(b);
         return &tb->value;
    }    
}; // end of template multitype_map<K>

} // end of namespace tinfra

#endif
