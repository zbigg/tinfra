#ifndef tinfra_proxy_h_included
#define tinfra_proxy_h_included

#include "tinfra/any.h"
#include "tinfra/assert.h"
#include <vector>
#include <iostream>

/*
    MSVS note, 
    
    the calling convention in MSVC is varying for different type 
    of returnType
    
    for sizeof(ReturnTYpe) <= 8, result is returned in eax:edx (AFAIR)
    and thus
        int method(int,int)
        generates ret 8 
    for sizeof(ReturnTYpe > 8 )
        int method(int, int, void* return_object_space)
        generates ret 12
        
    example assembly dump:
    
    method returning struct which size==20
        PUBLIC	??0foo_value@@QAE@XZ				; foo_value::foo_value
        PUBLIC	?method1@bar_interface_impl@@UAE?AUfoo_value@@HH@Z ; bar_interface_impl::method1
        ; Function compile flags: /Odtp
        _TEXT	SEGMENT
        _this$ = -8						; size = 4
        $T70891 = -4						; size = 4
        ___$ReturnUdt$ = 8					; size = 4
        ___formal$ = 12						; size = 4
        ___formal$ = 16						; size = 4
        ?method1@bar_interface_impl@@UAE?AUfoo_value@@HH@Z PROC	; bar_interface_impl::method1
        ; _this$ = ecx
        ; Line 74
                push	ebp
                mov	ebp, esp
                sub	esp, 8
                mov	DWORD PTR _this$[ebp], ecx
                mov	DWORD PTR $T70891[ebp], 0
                mov	ecx, DWORD PTR ___$ReturnUdt$[ebp]
                call	??0foo_value@@QAE@XZ
                mov	eax, DWORD PTR $T70891[ebp]
                or	eax, 1
                mov	DWORD PTR $T70891[ebp], eax
                mov	eax, DWORD PTR ___$ReturnUdt$[ebp]
                mov	esp, ebp
                pop	ebp
                ret	12					; 0000000cH
    method returning struct which size=4
        PUBLIC	?method1x@bar_interface_impl@@UAEHHH@Z		; bar_interface_impl::method1x
        ; Function compile flags: /Odtp
        _TEXT	SEGMENT
        _this$ = -4						; size = 4
        ___formal$ = 8						; size = 4
        ___formal$ = 12						; size = 4
        ?method1x@bar_interface_impl@@UAEHHH@Z PROC		; bar_interface_impl::method1x
        ; _this$ = ecx
        ; Line 75
                push	ebp
                mov	ebp, esp
                push	ecx
                mov	DWORD PTR _this$[ebp], ecx
                xor	eax, eax
                mov	esp, ebp
                pop	ebp
                ret	8
    my invokers:
        (return size=20)
            PUBLIC	??$invoke@$00HH@?$virtual_methods@Ufoo_value@@@tinfra@@SE?AUfoo_value@@PAXHH@Z ; tinfra::virtual_methods<foo_value>::invoke<1,int,int>
            (...)
            ret 8
        return size=4
            PUBLIC	??$invoke@$01HH@?$virtual_methods@H@tinfra@@SEHPAXHH@Z ; tinfra::virtual_methods<int>::invoke<2,int,int>
            (...)
            ret 8
    looks like we have to build trampolines by ourselves :/
*/
namespace tinfra {
    
class proxy_handler {
public:
    virtual ~proxy_handler() {};
    virtual any invoke(std::string const& method, std::vector<tinfra::any>& args) = 0;
};

struct proxy_object {
    void**         vtable;
    
    std::vector<std::string> names;
    proxy_handler* handler;
};

#ifdef _MSC_VER
const int VTABLE_INITIAL_IDX = 1;
#define TINFRA_PROXY_THISCALL __thiscall 
#define TINFRA_PROXY_GET_THIS(_orig, _xx) \
    _asm { mov _xx,ecx }; \
    if( sizeof(ReturnType) <= 8 ) { _xx = reinterpret_cast<proxy_object*>(_orig); }
#else
const int VTABLE_INITIAL_IDX = 2;
#define TINFRA_PROXY_THISCALL
#define TINFRA_PROXY_GET_THIS(_orig, proxy)
#endif 

class proxy_builder {
public:
    proxy_object make_proxy(proxy_handler* handler) ;
    
    template <int idx, typename CLS, typename R>
    void method(const char* name, R (TINFRA_PROXY_THISCALL CLS::*)());
    
    template <int idx, typename CLS, typename R, typename T>
    void method(const char* name, R (TINFRA_PROXY_THISCALL CLS::*)(T));

    template <int idx, typename CLS, typename R, typename T1,typename T2>
    void method(const char* name, R (TINFRA_PROXY_THISCALL CLS::*)(T1,T2));

    template <int idx, typename CLS, typename R, typename T1,typename T2, typename T3>
    void method(const char* name, R (TINFRA_PROXY_THISCALL CLS::*)(T1,T2,T3));

    template <int idx, typename CLS, typename R, typename T1,typename T2, typename T3, typename T4>
    void method(const char* name, R (TINFRA_PROXY_THISCALL CLS::*)(T1,T2,T3,T4));
private:
    std::vector<void*>  functions;
    std::vector<std::string> names;
};

//
// implementation detail
//

template <typename ReturnType>
struct virtual_methods {
    template <int MethodNumber>
    static ReturnType TINFRA_PROXY_THISCALL invoke(void* obj)
    {
        std::cout << "invoke0<" << MethodNumber << "> called on ptr=" << obj << "\n";
        proxy_object* proxy = reinterpret_cast<proxy_object*>(obj);
        
        TINFRA_ASSERT(MethodNumber+VTABLE_INITIAL_IDX < proxy->names.size());
        const std::string& name = proxy->names[MethodNumber+VTABLE_INITIAL_IDX];
        std::vector<any> args;
        
        std::cout << "invoke0<" << MethodNumber << ">("<<name<<": calling handler " << proxy->handler << "\n";
        any value = proxy->handler->invoke(name, args);
        std::cout << "invoke0<" << MethodNumber << ">: returned\n";
        return value.get<ReturnType>();
    }
    
    template <int MethodNumber, typename T1>
    static ReturnType TINFRA_PROXY_THISCALL invoke(void* obj, T1 v1)
    {
        std::cout << "invoke1<" << MethodNumber << "> called on ptr=" << obj << "\n";
        std::cout << "arg0: " << v1 << "\n";
        proxy_object* proxy = reinterpret_cast<proxy_object*>(obj);
        
        TINFRA_ASSERT(MethodNumber+VTABLE_INITIAL_IDX < proxy->names.size());
        const std::string& name = proxy->names[MethodNumber+VTABLE_INITIAL_IDX];
        std::vector<any> args;
        args.push_back(any::by_ref(v1)); // consider remove_ref
        
        std::cout << "invoke1<" << MethodNumber << ">("<<name<<": calling handler " << proxy->handler << "\n";
        any value = proxy->handler->invoke(name, args);
        return value.get<ReturnType>();
    }
    
    template <int MethodNumber, typename T1, typename T2>
    static ReturnType TINFRA_PROXY_THISCALL invoke(void* obj, T1 v1, T2 v2)
    {     
        proxy_object* proxy = reinterpret_cast<proxy_object*>(obj);
        TINFRA_PROXY_GET_THIS(obj, proxy);
        std::cout << "invoke2<" << MethodNumber << "> called on ptr=" << proxy << "\n";
        std::cout << "arg0: " << v1 << "\n";
        std::cout << "arg1: " << v2 << "\n";
        //return ReturnType();
        
        TINFRA_ASSERT(MethodNumber+VTABLE_INITIAL_IDX < proxy->names.size());
        const std::string& name = proxy->names[MethodNumber+VTABLE_INITIAL_IDX];
        std::vector<any> args;
        args.push_back(any::by_ref(v1)); // consider remove_ref
        args.push_back(any::by_ref(v2)); // consider remove_ref
        
        std::cout << "invoke2<" << MethodNumber << ">("<<name<<": calling handler " << proxy->handler << "\n";
        any value = proxy->handler->invoke(name, args);
        return value.get<ReturnType>();
    }

    template <int MethodNumber, typename T1, typename T2, typename T3>
    static ReturnType TINFRA_PROXY_THISCALL invoke(void* obj, T1 v1, T2 v2, T3 v3)
    {
        proxy_object* proxy = reinterpret_cast<proxy_object*>(obj);
        TINFRA_PROXY_GET_THIS(obj, proxy);
        
        std::cout << "invoke3<" << MethodNumber << "> called on ptr=" << proxy << "\n";
        std::cout << "arg0: " << v1 << "\n";
        std::cout << "arg1: " << v2 << "\n";
        std::cout << "arg2: " << v3 << "\n";
        
        TINFRA_ASSERT(MethodNumber+VTABLE_INITIAL_IDX < proxy->names.size());
        const std::string& name = proxy->names[MethodNumber+VTABLE_INITIAL_IDX];
        std::vector<any> args;
        args.push_back(any::by_ref(v1)); // consider remove_ref
        args.push_back(any::by_ref(v2)); // consider remove_ref
        args.push_back(any::by_ref(v3)); // consider remove_ref
        
        std::cout << "invoke3<" << MethodNumber << ">("<<name<<": calling handler " << proxy->handler << "\n";
        any value = proxy->handler->invoke(name, args);
        return value.get<ReturnType>();
    }
    
    template <int MethodNumber, typename T1, typename T2, typename T3, typename T4>
    static ReturnType TINFRA_PROXY_THISCALL invoke(void* obj, T1 v1, T2 v2, T3 v3, T4 v4)
    {
        proxy_object* proxy = reinterpret_cast<proxy_object*>(obj);
        TINFRA_PROXY_GET_THIS(obj, proxy);
        
        std::cout << "invoke4<" << MethodNumber << "> called on ptr=" << proxy << "\n";
        std::cout << "arg0: " << v1 << "\n";
        std::cout << "arg1: " << v2 << "\n";
        std::cout << "arg2: " << v3 << "\n";
        std::cout << "arg2: " << v4 << "\n";
        
        TINFRA_ASSERT(MethodNumber+VTABLE_INITIAL_IDX < proxy->names.size());
        const std::string& name = proxy->names[MethodNumber+VTABLE_INITIAL_IDX];
        std::vector<any> args;
        args.push_back(any::by_ref(v1)); // consider remove_ref
        args.push_back(any::by_ref(v2)); // consider remove_ref
        args.push_back(any::by_ref(v3)); // consider remove_ref
        args.push_back(any::by_ref(v4)); // consider remove_ref
        
        std::cout << "invoke4<" << MethodNumber << ">("<<name<<": calling handler " << proxy->handler << "\n";
        any value = proxy->handler->invoke(name, args);
        return value.get<ReturnType>();
    }

};

//
// proxy_builder
//
template <int idx, typename CLS, typename R>
void proxy_builder::method(const char* name, R (TINFRA_PROXY_THISCALL CLS::*)()) {
    names.push_back(name);
    R (TINFRA_PROXY_THISCALL *fun)(void*) = &virtual_methods<R>::template invoke<idx>;
    functions.push_back( (void*)fun );
}

template <int idx, typename CLS, typename R, typename T>
void proxy_builder::method(const char* name, R (TINFRA_PROXY_THISCALL CLS::*)(T)) {
    const int index = names.size();
    names.push_back(name);
    R (TINFRA_PROXY_THISCALL *fun)(void*, T) = &virtual_methods<R>::template invoke<idx, T>;
    functions.push_back( (void*)fun );
}

template <int idx, typename CLS, typename R, typename T1, typename T2>
void proxy_builder::method(const char* name, R (TINFRA_PROXY_THISCALL CLS::*)(T1,T2)) {
    const int index = names.size();
    names.push_back(name);
    R (TINFRA_PROXY_THISCALL *fun)(void*, T1,T2) = &virtual_methods<R>::template invoke<idx, T1, T2>;
    functions.push_back( (void*)fun );
}

template <int idx, typename CLS, typename R, typename T1, typename T2, typename T3>
void proxy_builder::method(const char* name, R (TINFRA_PROXY_THISCALL CLS::*)(T1,T2,T3)) {
    const int index = names.size();
    names.push_back(name);
    R (TINFRA_PROXY_THISCALL *fun)(void*, T1,T2,T3) = &virtual_methods<R>::template invoke<idx, T1, T2, T3>;
    functions.push_back( (void*)fun );
}

template <int idx, typename CLS, typename R, typename T1, typename T2, typename T3, typename T4>
void proxy_builder::method(const char* name, R (TINFRA_PROXY_THISCALL CLS::*)(T1,T2,T3,T4)) {
    const int index = names.size();
    names.push_back(name);
    R (TINFRA_PROXY_THISCALL *fun)(void*, T1,T2,T3, T4) = &virtual_methods<R>::template invoke<idx, T1, T2, T3, T4>;
    functions.push_back( (void*)fun );
}

inline
proxy_object proxy_builder::make_proxy(proxy_handler* handler) 
{
    proxy_object obj;
    // prepare tables
    size_t table_size = this->functions.size() + VTABLE_INITIAL_IDX;
    obj.vtable = new void*[table_size];
    memset(obj.vtable, 0, sizeof(void*)*table_size);
    
    obj.names.resize(table_size);
    
    for(unsigned i = 0; i < this->functions.size();i++ ) {
        obj.vtable[i+VTABLE_INITIAL_IDX] = this->functions[i];
        obj.names[i+VTABLE_INITIAL_IDX] = this->names[i];
    }
    obj.handler = handler;
    return obj;
}

} // end namespace tinfra

#endif // include guard
