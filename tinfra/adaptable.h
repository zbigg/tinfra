#ifndef tinfra_adaptable_h_included
#define tinfra_adaptable_h_included

#include "platform.h"

#include <map>
#include <typeinfo>
#include <cstdlib>

namespace tinfra {

class adaptable {
public:
    virtual ~adaptable();
    
    template <typename T>
    T& get_adapter();
    
    template <typename T>
    bool get_adapter(T*&);
private:
    virtual void* do_get_adapter(std::type_info const& ti) = 0;
};

class generic_adapter {
public:
    /*
    TODO: add support for parent adaptables
    
    generic_adapter():
        parent_(0)
    {}
        
    generic_adapter(adaptable* parent):
        parent_(0)
    {}
    */

    template <typename T>
    void add(T& t);

    void* get(std::type_info const& ti);
private:    
    struct type_info_wrapper {
        std::type_info const& ti_;
        
        explicit type_info_wrapper(std::type_info const& ti): ti_(ti) {}
            
        bool operator<(type_info_wrapper const& other) const {
            return ti_.before(other.ti_);
        }
    };
    void add(std::type_info const&, void* value);
    
    typedef std::map<type_info_wrapper, void*> adapter_map_t;
    adapter_map_t adapters;
};

#define TINFRA_DECLARE_ADAPTABLE                        \
private:                                                \
    void* do_get_adapter(std::type_info const& ti) {  \
        return this->tinfra_adapter.get(ti); \
    }                                                   \
protected:                                              \
    generic_adapter tinfra_adapter;


class generic_adaptable: public adaptable {
    TINFRA_DECLARE_ADAPTABLE
};

//
// template implementation
//
template <typename T>
T& adaptable::get_adapter() {
    void* raw_ptr = this->do_get_adapter(typeid(T));
    if( raw_ptr == 0 ) {
        abort();
    }
    T* typed = reinterpret_cast<T*>(raw_ptr);
    return *typed;
}

template <typename T>
bool adaptable::get_adapter(T*& result) {
    void* untyped = this->do_get_adapter(typeid(T));
    if( untyped == 0 )
        return false;
    result = reinterpret_cast<T*>(untyped);
    return true;
}

template <typename T>
void generic_adapter::add(T& t) {
    this->add(typeid(T), &t);
}

} // end namespace tinfra

#endif // tinfra_adaptable_h_included

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:
