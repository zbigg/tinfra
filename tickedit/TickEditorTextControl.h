#ifndef __TickEditorTextControl_h__
#define __TickEditorTextControl_h__

#include <wx/stc/stc.h>
#include <memory>

//
// TickEditorTextControl interface
//

class TickEditorTextControl: public wxStyledTextCtrl {
public:
    TickEditorTextControl();
    TickEditorTextControl(wxWindow *parent, wxWindowID id=wxID_ANY,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize, long style = 0,
               const wxString& name = wxT("TickEditorTextControl"));

    ~TickEditorTextControl();

    bool Create(wxWindow *parent, wxWindowID id=wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize, long style = 0,
                const wxString& name = wxT("TickEditorTextControl"));

    bool HasSelection();
private:
    void init();

    class Implementation;
    std::auto_ptr<Implementation> impl;

    DECLARE_DYNAMIC_CLASS(TickEditorTextControl);
};

#endif
