#include <allpch.h>

#include <wx/aui/aui.h>
#include <wx/toolbar.h>
#include <wx/artprov.h>

#include <wx/recguard.h>
//#include <tinfra/cmd.h>
#include <wx/mimetype.h>

#include <map>
#include <iostream>
#include <sstream>

#include "TickEditorTextControl.h"
#include "common.h"
#include "auto_loader.h"
#include "action.h"

WX_AUTO_LOADER(common_editor_commands)
{
    AM_ACTION(wxID_CUT,     "cut",    "Cu&t",     wxART_CUT ,     "CTRL+X");
    AM_ACTION(wxID_COPY,    "copy",   "&Copy",    wxART_COPY ,    "CTRL+C");
    AM_ACTION(wxID_PASTE,   "paste",  "&Paste",   wxART_PASTE,   "CTRL+V");
    AM_ACTION(wxID_DELETE,  "delete", "&Delete",  wxART_DELETE,  "DEL");
    AM_ACTION(wxID_FIND,    "find",   "&Find",    wxART_FIND,    "CTRL+F");
    AM_ACTION(wxID_REPLACE, "replace", "R&eplace",wxART_FIND_AND_REPLACE , "CTRL+H");  

    return true;
}

WX_AUTO_LOADER(common_document_commands)
{
    AM_ACTION(wxID_NEW,   "new",     "&New",    wxART_NEW ,     "CTRL+N");
    AM_ACTION(wxID_OPEN,  "open",    "&Open",   wxART_FILE_OPEN ,    "CTRL+O");
    AM_ACTION(wxID_REVERT,"revert",  "&Revert", "revert",      "CTRL+R");
    AM_ACTION(wxID_CLOSE, "close",   "&Close",  "fileclose",   "CTRL+W");
    AM_ACTION(wxID_SAVE,  "save",    "&Save",   wxART_FILE_SAVE,    "CTRL+S");
    AM_ACTION(wxID_SAVEAS,"save-as", "Save &as",wxART_FILE_SAVE_AS,  "CTRL+SHIFT+S");
    AM_ACTION(wxID_PRINT, "print",   "&Print",  wxART_PRINT ,   "CTRL+P");
    
    return true;
}

class TickEditApp: public wxApp {
public:
	/*
    void OnKeyEvent(wxKeyEvent& ev)
    {
        // don't care about keyp
        std:: cerr << "key: ";
        if( ev.ControlDown() )
            std::cerr << "CTRL+";
        if( ev.ShiftDown() )
            std::cerr << "SHIFT+";
        std::cerr << ev.GetKeyCode() << std::endl;
        ev.Skip();
    }
    
    bool ProcessEvent(wxEvent& ev)
    {
        wxCommandEvent* cmdev = dynamic_cast<wxCommandEvent*>(&ev);
        if( cmdev && (
            cmdev->GetEventType() == wxEVT_COMMAND_BUTTON_CLICKED || 
            cmdev->GetEventType() == wxEVT_COMMAND_MENU_SELECTED ) ) {
            std::cerr << "command " << toString(ev) <<  std::endl;
            
            static int fe = 10;
            
            if( --fe == 0 ) {
                char* p = 0;
                *p = 0;
            }
        }
        return wxApp::ProcessEvent(ev);
    }
	*/

	virtual bool OnInit();
protected:
    DECLARE_DYNAMIC_CLASS(TickEditApp);
    DECLARE_EVENT_TABLE();
};
IMPLEMENT_DYNAMIC_CLASS(TickEditApp,wxApp);

#ifdef TINFRA_MAIN
IMPLEMENT_APP_NO_MAIN(TickEditApp);
#else
IMPLEMENT_APP(TickEditApp);
#endif


BEGIN_EVENT_TABLE(TickEditApp, wxApp)
    //EVT_KEY_DOWN(TickEditApp::OnKeyEvent)    
END_EVENT_TABLE()

//
// action vs. toolbar helper
//
void addAction(wxToolBar* t, int id)
{
    ActionInfo* ai = ActionManager::getInstance()->getActionInfo(id);
    if( ! ai ) 
        return;
    
    wxBitmap bmp = wxArtProvider::GetBitmap(ai->icon_resource, wxART_TOOLBAR);
	if( !bmp.IsOk() )
		bmp = wxArtProvider::GetBitmap(wxART_ERROR , wxART_TOOLBAR);
    t->AddTool(ai->id, ai->label, bmp);
}

wxToolBar* buildDocumentToolbar(wxWindow* parent)
{
    wxToolBar* t = new wxToolBar(parent,-1);
    
    addAction(t, wxID_NEW);
    addAction(t, wxID_OPEN);
    addAction(t, wxID_SAVE);
    addAction(t, wxID_CLOSE);
    t->Realize();
    return t;
}

wxToolBar* buildEditorToolbar(wxWindow* parent)
{
    wxToolBar* t = new wxToolBar(parent,-1);
    
    addAction(t, wxID_CUT);
    addAction(t, wxID_COPY);
    addAction(t, wxID_PASTE);
    t->AddSeparator();
    addAction(t, wxID_FIND);
    addAction(t, wxID_REPLACE);
    t->Realize();
    return t;
}

class ControlCommandForwarder: public wxEvtHandler {
    wxWindow* wnd;
public:    
    ControlCommandForwarder(wxWindow* wnd): wnd(wnd) {}
        
    bool ProcessEvent(wxEvent& ev)
    {
        // guard against recursion
        static wxRecursionGuardFlag recursionFlag;        
        wxRecursionGuard guard(recursionFlag);
        
        if ( guard.IsInside() )
            return Skip(ev);

        // choose event type
        bool forward = false;
        int etype = ev.GetEventType();
        if( etype == wxEVT_UPDATE_UI ) {
            forward = true;
        } else if( etype == wxEVT_COMMAND_BUTTON_CLICKED 
                || etype == wxEVT_COMMAND_MENU_SELECTED 
                || etype == wxEVT_COMMAND_TOOL_CLICKED ) 
        {
            ev.SetEventType(wxEVT_COMMAND_MENU_SELECTED);
            forward = true;        
        }
        if( !forward )
            return Skip(ev);
        
        // send the event
        wxWindow* active = wxWindow::FindFocus(); // TODO: in fact it should be AUI active window
        

        if( ! active )
            return Skip(ev);
        
        bool handled = active->GetEventHandler()->ProcessEvent(ev);
        
        if( handled && dynamic_cast<wxUpdateUIEvent*>(&ev) == 0 ) 
            std::cerr << "ControlCommandForwarder -> " << toString(ev) << " to " << toString(active) << "HANDLED!\n";
        
        if( handled ) 
            return true;
        
        return Skip(ev);
    }
    
    bool Skip(wxEvent& ev)
    {
        if( GetNextHandler() ) 
            return GetNextHandler()->ProcessEvent(ev);
        else
            return wxEvtHandler::ProcessEvent(ev);
    }
};

#define LOG_INSPECT(a) do { std::cerr << __FILE__ << ":" << __LINE__ << ": " << #a << " -> " << a << std::endl; } while(0)

wxFrame* tickedit_init(wxWindow* parent)
{
	wxFrame* frame = new wxFrame(parent, -1, "Tickedit", 
            wxDefaultPosition, wxDefaultSize, 
            wxDEFAULT_FRAME_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX | wxMINIMIZE_BOX);
    {
        frame->PushEventHandler(new ControlCommandForwarder(frame));
        
        wxAuiManager* mgr = new wxAuiManager(frame,   wxAUI_MGR_ALLOW_ACTIVE_PANE | wxAUI_MGR_ALLOW_FLOATING | wxAUI_MGR_RECTANGLE_HINT );
        
        //wxAuiNotebook* editorHost = new wxAuiNotebook(frame);
        TickEditorTextControl* editorHost = new TickEditorTextControl(frame);
        TickEditorTextControl* notepad = new TickEditorTextControl(frame);
        
        mgr->AddPane(buildDocumentToolbar(frame), wxAuiPaneInfo().ToolbarPane().Top() );
        mgr->AddPane(buildEditorToolbar(frame), wxAuiPaneInfo().ToolbarPane().Top() );
        
        mgr->AddPane(editorHost,  wxAuiPaneInfo().CaptionVisible(false).Centre().Dockable(false).CloseButton(false) );        
        mgr->AddPane(notepad, wxAuiPaneInfo().Caption("Notepad").Bottom().CloseButton(false) );
        
        //editorHost->AddPage(new TickEditorTextControl(editorHost),"Editor1");
        //editorHost->AddPage(new TickEditorTextControl(editorHost),"Editor2");
        
        frame->Show(true);
        
        mgr->Update();
    }
    {
        wxMimeTypesManager m;
        wxFileType* ft = m.GetFileTypeFromExtension("svg");
        wxString description;
        if( ft->GetDescription(&description) )
            LOG_INSPECT(description);
        wxString mime_type;
        if( ft->GetMimeType(&mime_type) )
            LOG_INSPECT(mime_type);
        
        LOG_INSPECT(ft->GetOpenCommand("a.h"));
    }
    return frame;
}

bool TickEditApp::OnInit()
{
    wxFrame* f = tickedit_init(0);
    SetExitOnFrameDelete(true);
    SetTopWindow(f);
    f->Show(true);
    
    
    return true;
}
#ifdef TINFRA_MAIN
int tickedit_main(int argc, char** argv)
{
    wxEntryStart(argc, argv);
    
	wxFrame* f = tickedit_init(0);
	
    //wxTextCtrl* editor = new wxTextCtrl(frame,-1, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
    wxGetApp().SetExitOnFrameDelete(true);
    wxGetApp().SetTopWindow(frame);
	frame->Show(true);
    wxGetApp().MainLoop();
       
    frame->PopEventHandler(); // AUICommandHelper
    
    wxEntryCleanup();
    return 0;
}

TINFRA_MAIN(tickedit_main);
#endif
