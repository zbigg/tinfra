#ifndef tinfra_gui_context_h_included__
#define tinfra_gui_context_h_included__

namespace tinfra {
namespace gui {

    
class global_context {
    
public:
    global_context(int argc, char** argv);
    virtual ~global_context();
};

} } // end namespace tinfra::gui

#endif

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++

