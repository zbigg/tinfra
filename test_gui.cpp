//
// This is a Longest common subsequence problem solution
// as described 
//    http://en.wikipedia.org/wiki/Longest_common_subsequence_problem
//
// this is _naive_, recursive approach without memoization.
// proof of concept, checking if i can still program some algorithm
// DONT TRY IT IN PRODUCTION
//

#include "tinfra/cmd.h"

#include <vector>
#include <cassert>
#include <iostream>

/*
namespace std {
template <typename T>
ostream& operator <<(ostream& s, std::vector<T> const& v)
{
	if( v.size() == 0 ) {
		return s << "{}";
	}
	s << "{";
	for(size_t i = 0; i < v.size(); ++i ) {
		if( i > 0 )
			s << ",";
		s << v[i];
	}
	s << "}";
	return s;
}
}

template <typename T>
struct visualizator {
};

template <>
struct visualizator<std::ostream> {
	visualizator(std::ostream& out): out_(out) {}
	
	template <typename V>
	operator() (V const& v) {
		out << v;
	}
private:
	std::ostream& out_;
};
*/

#include "context.h"
#include <wx/dialog.h>

//namespace tg = tinfra::gui
namespace tg {
	using std::string;
	class container;
	class component;
	void add(container* where, container* what); 
	class component {
		parent_* parent;
	public:
		component(container* parent = 0):
			parent_(parent) 
		{
			add(parent, this);
		}
	
		component* parent() const { return this->parent_; }
	};
	
	class container: public component {	
		typedef std::vector<component*> collection_t;
		collection_t children_;
	public:
		container(container* parent = 0): component(parent) { }
		
		void add(component* c) { children_.push_back(c); }
		
		collection_t const& children() { return children_; }
	};
	
	class dialog: public container {
	public:
		dialog(string const& name);
	};
	
	void add(container* where, container* what)
	{
		where->add(what);
	}
};

static tg::dialog  about_box("about_box");
static tg::button  confirm_button(&about_box, "confirm_button");

int test_gui_main(int argc, char** argv)
{
	tinfra::gui::global_context gui(argc, argv);
	
	
	/*
	std:: X;
	X.push_back("A");
	X.push_back("G");
	X.push_back("H");
	X.push_back("C");
	X.push_back("D");
	X.push_back("E");
	X.push_back("B");
	X.push_back("T");
	
	
	seq Y;
	Y.push_back("G");
	Y.push_back("H");
	Y.push_back("A");
	Y.push_back("C");
	Y.push_back("D");
	Y.push_back("E");
	Y.push_back("T");
	*/
	
	wxDialog* dlg = new wxDialog(NULL, -1, wxT("tinfra::wxgui::test"), 
		wxDefaultPosition, wxDefaultSize, 
		wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX | wxMINIMIZE_BOX);

	dlg->ShowModal();
	
	delete dlg;
	return 0;
}

TINFRA_MAIN(test_gui_main);

