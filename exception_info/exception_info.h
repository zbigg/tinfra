#ifndef tinfra_exception_info_h_included
#define tinfra_exception_info_h_included

#include "tinfra/runtime.h" // for stacktrace_t

namespace tinfra {

struct exception_info {
    void*                 exception_object;
    const std::type_info* exception_type;
    stacktrace_t          throw_stacktrace;

    exception_info();
    
    static bool is_exception_active();
    static bool get_current_exception(exception_info& info);

};

void exception_info_callback(void *exception_object, std::type_info const* exception_type);

} // end namespace tinfra

#endif // tinfra_exception_info_h_included

