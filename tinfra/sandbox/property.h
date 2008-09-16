#ifndef __tinfra_property_h__
#define __tinfra_property_h__

namespace tinfra {

class base_property {
public:
    virtual tinfra::Symbol id() const = 0;
    virtual std::string str() const { std::string a; to_string(a); return a; }
    virtual void to_string(std::string& dest) const = 0;
    virtual void from_string(const char* src) = 0;
};
template <typename T>    
class property: public base_property {
    tinfra::Symbol _id;
    T _value;
    T _default_value;
public:
    basic_property(tinfra::Symbol const& id, T const& default_value): _id(id), _value(default_value), _default_value(default_value) {}
            
    T const& get() const { return _value; }
    T const& get_default() const { return _value; }
    void     set(T const& value) { _value = value; }
    void     set_default() { _value = _default_value; }
    
    virtual Symbol id() const { return _id; }
    virtual void to_string(std::string& dest) const { tinfra::to_string<T>(_value, dest); }
    virtual void from_string(const char* src) { tinfra::from_string<T>(src, _value); }
};

class property_registry {
public:
    // again we need type/runtime unversal map
};

};
};

#endif