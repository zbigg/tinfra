#include <windows.h>
#include <objbase.h>

#include <stdexcept>
#include <iostream>

#include "tinfra/cmd.h"
#include "tinfra/fmt.h"
#include "tinfra/win32.h"

using tinfra::fmt;
using std::string;

void fail(HRESULT hr, string const& message) {
    throw std::runtime_error(fmt("fatal error: %s (hr=%i)") % hr % message);
}

#define OGOSH_I_MAKE_NEXT_NEMU( stmt ) \
   do  {                               \
       HRESULT hr_ogosh = stmt;        \
       if( FAILED(hr_ogosh) )          \
           fail(hr_ogosh, #stmt);      \
   } while(0)

std::ostream& operator <<(std::ostream& out, CLSID const& clsid)
{
    
    return out << fmt("{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}") 
                    % clsid.Data1 
                    % clsid.Data2
                    % clsid.Data3
                    % (int)clsid.Data4[0] % (int)clsid.Data4[1]
                    % (int)clsid.Data4[2] % (int)clsid.Data4[3] 
                    % (int)clsid.Data4[4] % (int)clsid.Data4[5]
                    % (int)clsid.Data4[6] % (int)clsid.Data4[7];
}

template<typename T>
class com_ptr {
    T* ptr_;
    explicit com_ptr(T* ptr_to_be_actuired, bool inc_ref):
        ptr_(ptr_to_be_actuired)
    {
        if( inc_ref )
            ptr_->AddRef();
    }
public:
    static com_ptr<T> from_owned_reference(T* ptr) {
        return com_ptr(ptr, false);
    }
    
    static com_ptr<T> acquire_reference(T* ptr) {
        return com_ptr(ptr, true);
    }
    
    com_ptr():
        ptr_(0)
    {
    }
    com_ptr(com_ptr const& other):
        ptr_(other.ptr_)
    {
        if( ptr_ )
            ptr_->AddRef();
    }
    
    ~com_ptr() {
        reset();
    }
  
    com_ptr& operator=(com_ptr<T>& other) {
        if( this != &other ) {
            reset(other.ptr_);
        }
        return *this;
    }

    operator T*() { return ptr_; }
    
    T* operator->() { return ptr_; }
    
    T** as_out() {
        reset();
        return &ptr_;
    }
private:
    void reset(T* new_ptr = 0) {
        if( ptr_ )
                ptr_->Release();
        ptr_ = new_ptr;
        if( ptr_ )
            ptr_->AddRef();
    }
};

CLSID make_class_id(std::string const& object_name) 
{
    
    CLSID result;
    std::wstring wobject_name = tinfra::win32::make_wstring_from_utf8(object_name);
    HRESULT hr = CLSIDFromProgID(wobject_name.c_str(), &result);
    if( FAILED(hr) )
        fail(hr, fmt("CLSIDFromProgID(%s)") % object_name);
    
    return result;
}

template <typename T>
com_ptr<T> create_instance(CLSID const& clsid, IID const& iid)
{
    T* result;
    HRESULT hr = CoCreateInstance( clsid,
        NULL, CLSCTX_LOCAL_SERVER,
        iid,
        (void**)&result);
    
    if( FAILED(hr) ) {
        fail(hr, fmt("CoCreateInstance(clsid=%s, iid=%s") % clsid % iid);
    }
    return com_ptr<T>::from_owned_reference(result);
}
  


int excel_sample_main(int argc, char** argv)
{
    CoInitialize(NULL);
    CLSID excel_application_clsid = make_class_id("Excel.Application");
    
    std::cout << "Excel.Application=" << excel_application_clsid << "\n";
    
    com_ptr<IDispatch> edisp = create_instance<IDispatch>(excel_application_clsid, IID_IDispatch);
    
    com_ptr<ITypeInfo> iti;
    OGOSH_I_MAKE_NEXT_NEMU( edisp->GetTypeInfo(0, 0, iti.as_out()) );
    
    return 0;
}

TINFRA_MAIN(excel_sample_main);
