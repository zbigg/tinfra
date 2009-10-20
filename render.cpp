#include "model.h"

#include "controls/AdvDialog.h"

namespace tinfra { namespace  gui {

void show(dialog* definition, container* layout)
{
    AdvDialog* dlg = new AdvDialog(NULL, -1, wxT("tinfra::wxgui::test"), 
        wxDefaultPosition, wxDefaultSize, 
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX | wxMINIMIZE_BOX);

    dlg->ShowModal();

    delete dlg;
}

} } // end namespace tinfra::gui

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++

