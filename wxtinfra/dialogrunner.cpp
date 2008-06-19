#include <wx/app.h>
#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/stattext.h>

#include <tinfra/tinfra.h>

namespace S {
    tinfra::symbol street("street");
    tinfra::symbol house("house");
    tinfra::symbol city("city");
    tinfra::symbol postal_code("postal_code");
};

struct Address {
    std::string street;
    std::string house;
    
    std::string city;
    std::string postal_code;
    
    template <typename F>
        void apply(F& f) const {
            f(S::street, street);
            f(S::house, house);
            f(S::city, city);
            f(S::postal_code, postal_code);
        }
};

struct FormController {
    static wxWindow* findControl(wxWindow* parent, tinfra::symbol const& sym) {
	return wxWindow::FindWindowById(sym.id(), parent);
    }
};
template <typename T>
struct EditorController: public FormController {
	static wxControl* createEditorControl(wxWindow* parent, tinfra::symbol const& sym, T const& initialValue);
	static void setEditorControlValue(wxWindow* parent, tinfra::symbol const& sym, T const& value);
};

template<>
wxControl* EditorController<std::string>::createEditorControl(wxWindow* parent, tinfra::symbol const& sym, std::string const& initialValue)
{
	return new wxTextCtrl(parent, sym.id(), initialValue);
}

template<>
void EditorController<std::string>::setEditorControlValue(wxWindow* parent, tinfra::symbol const& sym, std::string const& value)
{
	wxWindow* wnd = findControl(parent, sym);
	if( !wnd ) abort();
	wxTextCtrl* textCtrl = dynamic_cast<wxTextCtrl*>(wnd);
	if( !textCtrl ) abort();
	
	textCtrl->SetValue(value);
}

struct FormRowBuilder {
	wxWindow* parent;
	
	FormRowBuilder(wxWindow* parent): parent(parent) {}
	
	template <typename T>
	void operator()(tinfra::symbol const& s, T const& v)
	{
		wxSizer* sizer = parent->GetSizer();
		
		wxStaticText* label = new wxStaticText(parent, -1, makeLabel(s));
		wxControl*    ctl   = EditorController<T>::createEditorControl(parent, s, v);
		
		sizer->Add(label, 0, wxALL | wxALIGN_CENTER_VERTICAL, 3);
		sizer->Add(ctl,   1, wxEXPAND | wxALL, 3);
	}
	
	wxString makeLabel(tinfra::symbol const& symbol)
	{
		return symbol.str();
	}
};

struct FormFiller {
	wxWindow* parent;
	
	FormFiller(wxWindow* parent): parent(parent) {}
	
	template <typename T>
	void operator()(tinfra::symbol const& s, T const& v)
	{		
		EditorController<T>::setEditorControlValue(parent, s, v);
	}
};

template <typename T>
struct FormManager {
	static wxWindow* buildForm(wxWindow* parent) 
	{
		wxPanel* panel = new wxPanel(parent);
		wxFlexGridSizer* sizer = new wxFlexGridSizer(0,2,2,2);
		sizer->AddGrowableCol(1);
		panel->SetSizer(sizer);
		
		T dummy;
		FormRowBuilder builder(panel);
		tinfra::process(dummy, builder);
		return panel;
	}
	
	static void fillForm(wxWindow* wnd, T const& data)
	{
		FormFiller filler(wnd);
		tinfra::process(data, filler);
	}
};
IMPLEMENT_APP_NO_MAIN(wxApp);

int main(int argc, char** argv)
{
	
	wxEntryStart(argc, argv);
	wxDialog* dlg = new wxDialog(NULL, -1, "hello", 
		wxDefaultPosition, wxDefaultSize, 
		wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX | wxMINIMIZE_BOX);
	wxWindow* form = FormManager<Address>::buildForm(dlg);
	
	Address a;
	a.street = "Strzelecka";
	a.house = "5g/11";
	a.city  = "Szczecin";
	a.postal_code = "72-004";
	FormManager<Address>::fillForm(form, a);
	
	wxSizer* s = new wxBoxSizer(wxVERTICAL);
	s->Add(form, 1, wxALL | wxEXPAND, 1);
	dlg->SetSizer(s);
	dlg->Fit();
    
	FormController::findControl(dlg, S::street)->SetFocus();
	dlg->ShowModal();
	wxEntryCleanup();
	return 0;
}

