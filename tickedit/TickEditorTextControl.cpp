#include <wx/wxprec.h>

#include "TickEditorTextControl.h"

#include "common.h"

//
// TickEditorTextControl public interface
//

TickEditorTextControl::TickEditorTextControl()
    : impl(0)
{
}

TickEditorTextControl::TickEditorTextControl(wxWindow *parent, wxWindowID id,
               const wxPoint& pos,
               const wxSize& size, long style,
               const wxString& name)
    : impl(0)
{
    init();
    Create(parent,id,pos,size,style,name);
}


TickEditorTextControl::~TickEditorTextControl()
{
    PopEventHandler();
}

bool TickEditorTextControl::Create(wxWindow *parent, wxWindowID id,
                const wxPoint& pos,
                const wxSize& size, long style,
                const wxString& name)
{
    return wxStyledTextCtrl::Create(parent, id, pos, size, style, name);
}

bool TickEditorTextControl::HasSelection()
{
    int start = GetSelectionStart();
    int end  = GetSelectionEnd();
    return (end-start) > 0;
}

//
// implementation details
//

IMPLEMENT_DYNAMIC_CLASS(TickEditorTextControl, wxStyledTextCtrl);

class TickEditorTextControl::Implementation: public wxEvtHandler {
    TickEditorTextControl* ctrl;
public:
    Implementation(): ctrl(0) {}
    Implementation(TickEditorTextControl* ctrl): ctrl(ctrl) {}
        
        
    void onEditorCommand(wxCommandEvent& ev)
    {
        std::cerr << "TickEditorTextControl: trying editor command\n";
        switch( ev.GetId() )  {
        case wxID_COPY:   ctrl->Copy(); return;
        case wxID_CUT:    ctrl->Cut(); return;
        case wxID_PASTE:  ctrl->Paste(); return;
        case wxID_DELETE: ctrl->Clear(); return;
        case wxID_SAVE:   
            {
                std::string open_folder = ".";
                std::string default_filename = "";
                std::string default_extension = "";
                wxString filename = wxFileSelector("Open a file", 
                                                   open_folder.c_str(), 
                                                   default_filename.c_str(), 
                                                   default_extension.c_str(), "*.*", 
                                                   wxFD_SAVE | wxFD_OVERWRITE_PROMPT, ctrl);
                if( !filename.IsEmpty() ) {
                    if( !ctrl->SaveFile(filename) ) {
                        std::cerr << "TickEditorTextControl: file save failed" << std::endl;
                    }
                }
                break;
            }
        
        default:
            ev.Skip();
        }
    }
    
    void onEditorCommandUI(wxUpdateUIEvent& ev)
    {    
        switch( ev.GetId() )  {
        case wxID_COPY:   ev.Enable(ctrl->HasSelection()); return;
        case wxID_CUT:    ev.Enable(ctrl->HasSelection()); return;
        case wxID_PASTE:  ev.Enable(ctrl->CanPaste()); return;
        case wxID_DELETE: ev.Enable(ctrl->HasSelection()); return;
        case wxID_SAVE:   ev.Enable(ctrl->GetModify()); return;
        default:
            ev.Skip();
        }
    }
    
    //~ bool ProcessEvent(wxEvent& ev)
    //~ {
        //~ //std::cerr << "TickEditorTextControl::Impl event: << " << toString(ev) << "\n";
        //~ return wxEvtHandler::ProcessEvent(ev);
    //~ }
private:
    DECLARE_EVENT_TABLE();
    DECLARE_DYNAMIC_CLASS(TickEditorTextControl::Implementation);
};

IMPLEMENT_DYNAMIC_CLASS(TickEditorTextControl::Implementation, wxEvtHandler);

BEGIN_EVENT_TABLE(TickEditorTextControl::Implementation, wxEvtHandler)
    EVT_MENU(wxID_COPY, TickEditorTextControl::Implementation::onEditorCommand)
    EVT_MENU(wxID_CUT, TickEditorTextControl::Implementation::onEditorCommand)
    EVT_MENU(wxID_PASTE, TickEditorTextControl::Implementation::onEditorCommand)
    EVT_MENU(wxID_DELETE, TickEditorTextControl::Implementation::onEditorCommand)
    EVT_MENU(wxID_SAVE, TickEditorTextControl::Implementation::onEditorCommand)

    EVT_UPDATE_UI(wxID_COPY, TickEditorTextControl::Implementation::onEditorCommandUI)
    EVT_UPDATE_UI(wxID_CUT, TickEditorTextControl::Implementation::onEditorCommandUI)
    EVT_UPDATE_UI(wxID_PASTE, TickEditorTextControl::Implementation::onEditorCommandUI)
    EVT_UPDATE_UI(wxID_DELETE, TickEditorTextControl::Implementation::onEditorCommandUI)
    EVT_UPDATE_UI(wxID_SAVE, TickEditorTextControl::Implementation::onEditorCommandUI)
END_EVENT_TABLE()

// this is at the end to know implementation of TickEditorTextControl::Implementation
void TickEditorTextControl::init()
{
    impl = std::auto_ptr<Implementation>(new Implementation(this));
    
    PushEventHandler(impl.get());
}
