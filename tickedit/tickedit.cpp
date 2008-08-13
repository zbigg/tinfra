#include <wx/app.h>
#include <wx/frame.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/stc/stc.h>
#include <wx/aui/aui.h>
#include <wx/toolbar.h>
#include <wx/artprov.h>
#include <tinfra/cmd.h>

#include <map>
#include <iostream>

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

    bool Create(wxWindow *parent, wxWindowID id=wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize, long style = 0,
                const wxString& name = wxT("TickEditorTextControl"));

private:
    void init();
};

//
//
//

struct ActionInfo {
    int id;
    std::string name;
    std::string label;
    std::string icon_resource;
    std::string shortcut;
};

class ActionManager {
public:
    static ActionManager* getInstance();

    ActionInfo* registerAction(int id,
                               std::string const& name, 
                               std::string const& label, 
                               std::string const& icon_resource = "",
                               std::string const& shortcut = "");

    ActionInfo* registerAction(std::string const& name, 
                               std::string const& label, 
                               std::string const& icon_resource = "",
                               std::string const& shortcut = "");
    
    ActionInfo* getActionInfo(int id);
    ActionInfo* getActionInfo(std::string const& name);
protected:
    ~ActionManager();
    ActionManager();

private:
    typedef std::map<std::string, ActionInfo*> ActionNameMap;
    typedef std::map<int, ActionInfo*>         ActionIdMap;

    ActionNameMap byNames;
    ActionIdMap   byIds;
};

ActionManager* ActionManager::getInstance()
{
    static ActionManager instance;
    return &instance;
}

ActionManager::ActionManager()
{
}

ActionManager::~ActionManager()
{
    for( ActionIdMap::const_iterator i = byIds.begin(); i != byIds.end(); ++i ) 
    {
        delete i->second;
    }
}


ActionInfo* ActionManager::registerAction(int id,
                         std::string const& name, 
                         std::string const& label, 
                         std::string const& icon_resource,
                         std::string const& shortcut)
{
    ActionInfo* ai = new ActionInfo();
    ai->id = id;
    ai->name = name;
    ai->label = label;
    ai->icon_resource = icon_resource;
    ai->shortcut = shortcut;
    
    byIds[id] = ai;
    byNames[name] = ai;
    return ai;
}

ActionInfo* ActionManager::registerAction(std::string const& name, 
                         std::string const& label, 
                         std::string const& icon_resource,
                         std::string const& shortcut)
{
    return registerAction(wxNewId(), name, label, icon_resource, shortcut);
}
    
ActionInfo* ActionManager::getActionInfo(int id)
{
    ActionIdMap::const_iterator a = byIds.find(id);
    if( a != byIds.end() )
        return a->second;
    else
        return 0;
}

ActionInfo* ActionManager::getActionInfo(std::string const& name)
{
    ActionNameMap::const_iterator a = byNames.find(name);
    if( a != byNames.end() )
        return a->second;
    else
        return 0;
}

#define AM_ACTION(id, name, label, icon_resource, shortcut) ActionManager::getInstance()->registerAction(id, name, label, icon_resource, shortcut)
#define AM_ACTION_AUTO(name, label, icon_resource, shortcut) ActionManager::getInstance()->registerAction(name, label, icon_resource, shortcut)
//
// TickEditorTextControl implementation
//

TickEditorTextControl::TickEditorTextControl(wxWindow *parent, wxWindowID id,
               const wxPoint& pos,
               const wxSize& size, long style,
               const wxString& name)
: wxStyledTextCtrl(parent, id, pos, size, style, name)
{
    init();
}

void TickEditorTextControl::init()
{
}



bool TickEditorTextControl::Create(wxWindow *parent, wxWindowID id,
                const wxPoint& pos,
                const wxSize& size, long style,
                const wxString& name)
{
    return wxStyledTextCtrl::Create(parent, id, pos, size, style, name);
}

#define AUTO_LOADER(name)                       \
class name##Loader: public wxModule {           \
    public: virtual bool OnInit();              \
    public: virtual void OnExit() {}            \
    private:                                    \
    DECLARE_DYNAMIC_CLASS(name##Loader); };     \
IMPLEMENT_DYNAMIC_CLASS(name##Loader, wxModule); \
bool name##Loader::OnInit() 

AUTO_LOADER(EditorCommands)
{
    AM_ACTION(wxID_CUT,     "cut",    "Cu&t",     "edit-cut",     "CTRL+X");
    AM_ACTION(wxID_COPY,    "copy",   "&Copy",    "edit-copy",    "CTRL+C");
    AM_ACTION(wxID_PASTE,   "paste",  "&Paste",   "edit-paste",   "CTRL+V");
    AM_ACTION(wxID_DELETE,  "delete", "&Delete",  "edit-delete",  "DEL");
    AM_ACTION(wxID_DELETE,  "find",   "&Find",    "edit-find",    "CTRL+F");
    AM_ACTION(wxID_REPLACE, "replace", "R&eplace","edit-find-replace", "CTRL+H");  

    return true;
}

AUTO_LOADER(DocumentCommands)
{
    AM_ACTION(wxID_NEW,   "new",     "&New",    "filenew",     "CTRL+N");
    AM_ACTION(wxID_OPEN,  "open",    "&Open",   "fileopen",    "CTRL+O");
    AM_ACTION(wxID_REVERT,"revert",  "&Revert", "revert",      "CTRL+R");
    AM_ACTION(wxID_CLOSE, "close",   "&Close",  "fileclose",   "CTRL+W");
    AM_ACTION(wxID_SAVE,  "save",    "&Save",   "filesave",    "CTRL+S");
    AM_ACTION(wxID_SAVEAS,"save-as", "Save &as","filesaveas", "CTRL+SHIFT+S");
    AM_ACTION(wxID_PRINT, "print",   "&Print",  "fileprint",   "CTRL+P");
    
    return true;
}

class TickEditApp: public wxApp {
public:
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
            std::cerr << "command " << ev.GetClassInfo()->GetClassName() << " id=" << ev.GetId() << std::endl;
                
            static int fe = 10;
            
            if( --fe == 0 ) {
                char* p = 0;
                *p = 0;
            }
        }
        return wxApp::ProcessEvent(ev);
    }
protected:
    DECLARE_DYNAMIC_CLASS(TickEditApp);
    DECLARE_EVENT_TABLE();
};
IMPLEMENT_DYNAMIC_CLASS(TickEditApp,wxApp);
IMPLEMENT_APP_NO_MAIN(TickEditApp);

BEGIN_EVENT_TABLE(TickEditApp, wxApp)
    EVT_KEY_DOWN(TickEditApp::OnKeyEvent)    
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
    t->AddTool(ai->id, ai->label, bmp);
}

wxToolBar* buildDocumentToolbar(wxWindow* parent)
{
    wxToolBar* t = new wxToolBar(parent,-1);
    
    addAction(t, wxID_NEW);
    addAction(t, wxID_OPEN);
    addAction(t, wxID_SAVE);
    addAction(t, wxID_CLOSE);
    
    return t;
}

wxToolBar* buildEditorToolbar(wxWindow* parent)
{
    wxToolBar* t = new wxToolBar(parent,-1);
    
    addAction(t, wxID_CUT);
    addAction(t, wxID_COPY);
    addAction(t, wxID_PASTE);
    addAction(t, wxID_FIND);
    addAction(t, wxID_REPLACE);
    
    return t;
}


int tickedit_main(int argc, char** argv)
{
    wxEntryStart(argc, argv);
    
    wxFrame* frame = new wxFrame(NULL, -1, "Tickedit", 
            wxDefaultPosition, wxDefaultSize, 
            wxDEFAULT_FRAME_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX | wxMINIMIZE_BOX);
    {
        wxAuiManager mgr(frame,   wxAUI_MGR_ALLOW_ACTIVE_PANE | wxAUI_MGR_ALLOW_FLOATING | wxAUI_MGR_RECTANGLE_HINT );
        wxAuiNotebook* editorHost = new wxAuiNotebook(frame);        
        TickEditorTextControl* notepad = new TickEditorTextControl(frame);
        
        mgr.AddPane(buildDocumentToolbar(frame), wxAuiPaneInfo().ToolbarPane().Top() );
        mgr.AddPane(buildEditorToolbar(frame), wxAuiPaneInfo().ToolbarPane().Top() );
        
        mgr.AddPane(editorHost,  wxAuiPaneInfo().CaptionVisible(false).Centre().Dockable(false).CloseButton(false) );        
        mgr.AddPane(notepad, wxAuiPaneInfo().Caption("Notepad").Bottom().CloseButton(false) );
        
        editorHost->AddPage(new TickEditorTextControl(editorHost),"Editor1");
        editorHost->AddPage(new TickEditorTextControl(editorHost),"Editor2");
        
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
