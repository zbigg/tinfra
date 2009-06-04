#ifndef tinfra_gui_model_h__
#define tinfra_gui_model_h__

#include <string>
#include <cassert>
#include <vector>

namespace tinfra { 
namespace gui {
    
using std::string;
class container;
class component;

void add(container* where, component* what); 

class component {
    container* parent_;
public:
    component(container* parent = 0):
        parent_(parent) 
    {
        if( parent != 0 ) {
            add(parent, this);
        }
    }

    container* parent() const { 
        return this->parent_; 
    }
};

class container: public component {    
    typedef std::vector<component*> collection_t;
    collection_t children_;
public:
    container(container* parent = 0): component(parent) { }
    
    void add(component* c) { children_.push_back(c); }
    
    collection_t const& children() { return children_; }
};

struct label: public component {
    label(string const& text, string const& icon = ""):
        component(0),
        text(text),
        icon(icon)
    {}
    
    string text;
    string icon;
};

class dialog: public container {
public:
    dialog(string const& title, string const& icon=""):
        caption(title, icon)
    {}

    label caption;
};

struct button: public component {
    button(container* parent, string const& title, string const& icon=""):
        component(parent),
        synopsis(title, icon)
    {}
        
    label synopsis;
};

inline void add(container* where, component* what)
{
    where->add(what);
}

} } // end namespace tinfra::gui

#endif
