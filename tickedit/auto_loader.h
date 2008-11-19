#ifndef __tinfra_wx_auto_loader_h__
#define __tinfra_wx_auto_loader_h__

#include <wx/module.h>

#define WX_AUTO_LOADER(name)                      \
class name##_loader: public wxModule {            \
    public: virtual bool OnInit();                \
    public: virtual void OnExit() {}              \
    private:                                      \
    DECLARE_DYNAMIC_CLASS(name##_loader); };      \
IMPLEMENT_DYNAMIC_CLASS(name##_loader, wxModule); \
bool name##_loader::OnInit() 


#endif // __tinfra_wx_auto_loader_h__
