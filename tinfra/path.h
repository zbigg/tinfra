#ifndef __tinfra_path__
#define __tinfra_path__

#include <string>

namespace tinfra { 
namespace path {
    
std::string join(const std::string& a, const std::string b);
    
bool exists(const char* name);
    
inline 
bool exists(const std::string& name) { return exists(name.c_str()); }


bool is_dir(const char* name);

std::string basename(const std::string& name);

std::string dirname(const std::string& name);

} } // end namespace tinfra::path

#endif
