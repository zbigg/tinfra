#ifndef tinfra_mo_interface_h_included
#define tinfra_mo_interface_h_included

namespace tinfra {

/// visit each interface method signature
///
/// a method Functor::method(const char* name, RET (CLS::*)(T...)
/// is called for each method exported by interface
/// a class being introspected must have manifest
/// defined using TINFRA_BEAN_MANIFEST 
template <typename T, typename Functor>
void visit_interface(Functor& f);
    

/// declare manifest of interface
/// // in root namespace
/// TINFRA_BEAN_MANIFEST(MyClass) {
///    TINFRA_BEAN_METHOD(MyClass, myMethod)
/// }
#define TINFRA_BEAN_MANIFEST(cls) TINFRA_BEAN_MANIFEST_IMPL(cls)
#define TINFRA_BEAN_METHOD(cls,name) TINFRA_BEAN_METHOD_IMPL(cls, name)

} // end namespace tinfra

//
// implementation
// consider moving into mo_interface_detail.h
//

template <typename T>
struct tinfra_interface_visit_traits {
    //template <typename Functor>                
    //static void xvisit_interface(Functor& f) {} 
};

#define TINFRA_BEAN_MANIFEST_IMPL(cls) \
    template <> \
    struct tinfra_interface_visit_traits<cls> {           \
        template <typename Functor>                \
        static void visit_interface(Functor& f);   \
    };                                      \
    template <typename Functor>              \
    void tinfra_interface_visit_traits<cls>::visit_interface(Functor& f)
    
#define TINFRA_BEAN_METHOD_IMPL(cls,name) \
    f.method(#name, &cls::name)

namespace tinfra {
    
template <typename T, typename Functor>
void visit_interface(Functor& f)
{
    ::tinfra_interface_visit_traits<T>::visit_interface(f);
}

} // end namespace tinfra

#endif
