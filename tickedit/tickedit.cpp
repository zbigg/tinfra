#include <wx/app.h>
#include <wx/frame.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/stc/stc.h>
#include <wx/aui/aui.h>
#include <tinfra/cmd.h>
//
// TickEditor interface
//

class TickEditor: public wxStyledTextCtrl {
public:
    TickEditor();
    TickEditor(wxWindow *parent, wxWindowID id=wxID_ANY,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize, long style = 0,
               const wxString& name = wxT("TickEditor"));

    bool Create(wxWindow *parent, wxWindowID id=wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize, long style = 0,
                const wxString& name = wxT("TickEditor"));

private:
    void init();
};

//
// TickEditor implementation
//
TickEditor::TickEditor()
{
    init();
}

TickEditor::TickEditor(wxWindow *parent, wxWindowID id,
               const wxPoint& pos,
               const wxSize& size, long style,
               const wxString& name)
: wxStyledTextCtrl(parent, id, pos, size, style, name)
{
    init();
}

void TickEditor::init()
{
}

bool TickEditor::Create(wxWindow *parent, wxWindowID id,
                const wxPoint& pos,
                const wxSize& size, long style,
                const wxString& name)
{
    return wxStyledTextCtrl::Create(parent, id, pos, size, style, name);
}

IMPLEMENT_APP_NO_MAIN(wxApp);

int tickedit_main(int argc, char** argv)
{
    wxEntryStart(argc, argv);
    
    wxFrame* frame = new wxFrame(NULL, -1, "Tickedit", 
            wxDefaultPosition, wxDefaultSize, 
            wxDEFAULT_FRAME_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX | wxMINIMIZE_BOX);
    {
        wxAuiManager mgr(frame,   wxAUI_MGR_ALLOW_ACTIVE_PANE | wxAUI_MGR_ALLOW_FLOATING | wxAUI_MGR_RECTANGLE_HINT );
        TickEditor* editor = new TickEditor(frame);        
        TickEditor* notepad = new TickEditor(frame);
        mgr.AddPane(editor,  wxAuiPaneInfo().Caption("TickEdit").Centre().Dockable(false).CloseButton(false) );
        mgr.AddPane(notepad, wxAuiPaneInfo().Caption("Notepad").Bottom().CloseButton(false) );
        
        frame->Show(true);
        mgr.Update();
        //wxTextCtrl* editor = new wxTextCtrl(frame,-1, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
        wxGetApp().SetExitOnFrameDelete(true);
        wxGetApp().SetTopWindow(frame);
        wxGetApp().MainLoop();
    }
    wxEntryCleanup();
    return 0;
}

TINFRA_MAIN(tickedit_main);
