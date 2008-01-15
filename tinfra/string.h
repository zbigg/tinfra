#ifndef __tinfra_string_h__
#define __tinfra_string_h__

#include <string>

namespace tinfra {

void        escape_c_inplace(std::string& a);
std::string escape_c(const std::string& a);
    
void        strip_inplace(std::string& s);
std::string strip(const std::string& s);

}

#endif
