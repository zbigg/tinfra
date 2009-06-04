#include "model.h"

#include <wx/dialog.h>

namespace tinfra { namespace  gui {

void show(dialog* definition, container* layout)
{
    wxDialog* dlg = new wxDialog(NULL, -1, wxT("tinfra::wxgui::test"), 
        wxDefaultPosition, wxDefaultSize, 
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX | wxMINIMIZE_BOX);

    dlg->ShowModal();

    delete dlg;
}

} } // end namespace tinfra::gui
