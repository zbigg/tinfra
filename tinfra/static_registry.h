/// automatic static container
///
/// This template dly defines facility to create and control static 
/// (static as static class member) registry of static objects.
/// The list is filled during initialization of process, before
/// actual main execution.
///
/// (Must borrow implementation from option, trace)

#ifndef tinfra_static_registry_h_included
#define tinfra_static_registry_h_included

#include <vector>

namespace tinfra {

//
// public interface
//    

template <typename T>
class static_registry {
public:
    static void register_element(T* instance);
    static void unregister_element(T* instance);

    static std::vector<T*> const& elements() {
        return get_element_container();
    }
private:
    
    static std::vector<T*>& get_element_container();
};

template <typename T>
class static_registry_element {
public:
    static_registry_element()
    {
        static_registry<T>::register_element(static_cast<T*>(this));
    }
    ~static_registry_element()
    {
        static_registry<T>::unregister_element(static_cast<T*>(this));
    }
};


//
// implementation details
//

template <typename T>
std::vector<T*>& static_registry<T>::get_element_container()
{
    static std::vector<T*> the_elements;
    return the_elements;
}

template <typename T>
void static_registry<T>::register_element(T* instance)
{
    get_element_container().push_back(instance);
}

template <typename T>
void static_registry<T>::unregister_element(T*)
{
    // TODO: implement!
}

} // end namespace tinfra

#endif 
