#ifndef __tickedit_common_h__
#define __tickedit_common_h__

#include <sstream>

static std::string toString(wxEvent& ev) 
{
    std::ostringstream b;
    b << "" << ev.GetClassInfo()->GetClassName() << "(id=" << ev.GetId() << ")";
    return b.str();
}

static std::string toString(wxWindow* w) 
{
    std::ostringstream b;
    b << "" << w->GetClassInfo()->GetClassName() << "(id=" << w->GetId() << ", name=" << w->GetName() << ")";
    return b.str();
}

#endif
