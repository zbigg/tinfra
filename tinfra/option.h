//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_option_h_included
#define tinfra_option_h_included

#include "tinfra/tstring.h"
#include <vector>
#include <sstream>

namespace tinfra {

class option_list;
class option_base {
public:
    static const char NO_SWITCH;

    option_base();
    option_base(option_list& pl);
    virtual ~option_base();
    
    virtual bool needs_argument() const = 0;
    virtual void accept(tstring const& value) = 0;
    
    virtual char        get_switch_letter() const = 0;
    virtual std::string get_name() const = 0;
    
    virtual std::string get_synopsis() const = 0;
    
    virtual std::string value_as_string() const = 0;
    virtual std::string default_as_string() const = 0;
};

class option_list {
public:
    typedef std::vector<option_base*> option_list_t;
    

    // mutation
    void add(option_base* opt);

    // processing
    void parse(std::vector<tstring>& params);
    
    void print_help(std::ostream& out);

    // query
    option_list_t get_options();
    option_base*  find_by_name(tstring const& name);
    option_base*  find_by_shortcut(char c);
private:
    option_list_t options;
};

class option_registry {
public:
    static option_list& get();     
};

class option_impl: public option_base {
public:
    static const char NO_SWITCH = 0;

    option_impl(option_list& ol, char switch_letter, std::string const& name, std::string const& synopsis);
    option_impl(option_list& ol, std::string const& name, std::string const& synopsis);

    option_impl(char switch_letter, std::string const& name, std::string const& synopsis);
    option_impl(std::string const& name, std::string const& synopsis);
    
    virtual char        get_switch_letter() const;
    
    virtual std::string get_name() const;
    
    virtual std::string get_synopsis() const;

protected:
    char        switch_letter;
    std::string name;
    std::string synopsis;
};

template <typename T>
class option: public option_impl {
public:
    option(option_list& ol, T const& default_value, char switch_letter, std::string const& name, std::string const& synopsis): 
        option_impl(ol, switch_letter, name, synopsis),
        default_value_(default_value),
        accepted_(false)
    {}
    
    option(option_list& ol, T const& default_value, std::string const& name, std::string const& synopsis): 
        option_impl(ol, name, synopsis),
        default_value_(default_value),
        accepted_(false)
    {}

    option(T const& default_value, char switch_letter, std::string const& name, std::string const& synopsis): 
        option_impl(switch_letter, name, synopsis),
        default_value_(default_value),
        accepted_(false)
    {}
    
    option(T const& default_value, std::string const& name, std::string const& synopsis): 
        option_impl(name, synopsis),
        default_value_(default_value),
        accepted_(false)
    {}
public:
    virtual bool needs_argument() const {
        return true;
    }
    
    virtual void accept(tstring const& value)
    {
        std::istringstream fmt(value.str());
        T tmp_val;
        fmt >> tmp_val;
        // TODO: check for correctnes of input
        //if( !fmt ) {
        //    return;
        //}
        accepted_ = true;
        using std::swap;
        swap(value_,tmp_val);
    }
    
    virtual std::string value_as_string() const {
        if( accepted_ ) {
            std::ostringstream tmp;
            tmp << value_;
            return tmp.str();
        } else {
            return default_as_string();
        }
    }
    
    virtual std::string default_as_string() const {
        std::ostringstream tmp;
        tmp << default_value_;
        return tmp.str();
    }
    
    bool accepted() const {
        return accepted_;
    }
    
    T const& value() const {
        if( ! accepted_ )
            return default_value_;
        return value_;
    }
    
    T const& default_value() const {
        return default_value_;
    }
private:
    T default_value_;
protected:
    T value_;
    bool accepted_;
};

class option_switch: public option<bool> {
public:
    option_switch(option_list& ol, char switch_letter, std::string const& name, std::string const& synopsis): 
        option<bool>(ol, false, switch_letter, name, synopsis)
    {}
    
    option_switch(option_list& ol, std::string const& name, std::string const& synopsis): 
        option<bool>(ol, false, name, synopsis)
    {}

    option_switch(char switch_letter, std::string const& name, std::string const& synopsis): 
        option<bool>(false, switch_letter, name, synopsis)
    {}
    
    option_switch(std::string const& name, std::string const& synopsis): 
        option<bool>(false, name, synopsis)
    {}
public:
    virtual bool needs_argument() const {
        return false;
    }
    virtual void accept(tstring const& value);
    
    bool enabled() const {
        return value() == true;
    }
    
    bool disabled() const {
        return value() == false;
    }
};

template <typename T>
class list_option: public option_impl {
public:
    list_option(option_list& ol, char switch_letter, std::string const& name, std::string const& synopsis): 
        option_impl(ol, switch_letter, name, synopsis),
        accepted_(false)
    {}
    
    list_option(option_list& ol, std::string const& name, std::string const& synopsis): 
        option_impl(ol, name, synopsis),
        accepted_(false)
    {}

    list_option(char switch_letter, std::string const& name, std::string const& synopsis): 
        option_impl(switch_letter, name, synopsis),
        accepted_(false)
    {}
    
    list_option(std::string const& name, std::string const& synopsis): 
        option_impl(name, synopsis),
        accepted_(false)
    {}

    virtual void accept(tstring const& value)
    {
        std::istringstream fmt(value.str());
        T tmp_val;
        fmt >> tmp_val;
        // TODO: check for correctnes of input
        //if( !fmt ) {
        //    return;
        //}
        accepted_ = true;
        value_.push_back(tmp_val);
    }
    
    virtual std::string value_as_string() const {
        if( accepted_ ) {
            std::ostringstream tmp;
            for(int i = 0; i < value_.size(); ++i ) {
                if( i > 0 )
                    tmp << ", ";
                tmp << value_[i];
            }
            return tmp.str();
        } else {
            return default_as_string();
        }
    }
    
    virtual std::string default_as_string() const {
        return "(none)";
    }
    
    virtual bool needs_argument() const {
        return true;
    }
    
    std::vector<T> const& value() const {
        return value_;
    }
    
    std::vector<T> default_value() const {
        return std::vector<T>();
    }
    
    bool accepted() const {
        return accepted_;
    }
private:
    std::vector<T> value_;
    bool           accepted_;
};

} // end namespace tinfra

#endif // tinfra_option_h_included

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

