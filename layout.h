#ifndef tinfra_gui_layout_h__
#define tinfra_gui_layout_h__

#include <string>
#include <cassert>
#include <vector>

#include "model.h"

namespace tinfra { 
namespace gui {

class  layout_element: public component {
public:
    layout_element(container* parent, component* element)
    {
        assert(parent != 0);
        parent->add(element);
    }
};

class  box: public container {
public:
    box(container* parent = 0):
        container(parent)
    {}
};

class  vertical_box: public box {
public:
    vertical_box(container* parent = 0):
        box(parent)
    {}
};

class  horizontal_box: public box {
public:
    horizontal_box(container* parent = 0):
        box(parent)
    {}
};

class  grid_box: public box {
public:
    grid_box(int rows, int cols, container* parent = 0):
        box(parent),
        rows(rows),
        cols(cols)
    {}
        
    int rows;
    int cols;
};

} } // end namespace tinfra::gui

#endif // tinfra_gui_layout_h__

