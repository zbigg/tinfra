//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_typeinfo_h_included
#define tinfra_typeinfo_h_included


#include "tstring.h"
#include <typeinfo>
#include <string>

namespace tinfra {

std::string demangle_type_info_name(const std::type_info& ti);

/// Get type name as string.
///
/// Note:
///   On GCC, this name is demangled using abi::__cxa_demangle
///   On other "platforms" this name equals to typeid(T).name()
///
template <typename T>
std::string type_name() {
    const std::type_info& ti = typeid(T);
    return demangle_type_info_name(ti);
}

} // end namespace tinfra

#endif // tinfra_typeinfo_h_included
