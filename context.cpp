#include <wx/app.h>

#include "context.h"

IMPLEMENT_APP_NO_MAIN(wxApp);

namespace tinfra { namespace gui {
    
global_context::global_context(int argc, char** argv)
{
    wxEntryStart(argc, argv);
}
global_context::~global_context()
{
    wxEntryCleanup();
}

} } // end namespace tinfra::gui

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++

