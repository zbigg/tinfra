#ifndef __tinfra_path__
#define __tinfra_path__

#include <string>
#include "tinfra/tstring.h"

namespace tinfra { 
namespace path {
    
std::string join(tstring const& a, tstring const& b);
    
bool exists(tstring const& name);

bool is_file(tstring const& name);
bool is_dir(tstring const& name);

std::string basename(tstring const& name);

std::string dirname(tstring const& name);

std::string tmppath();

} } // end namespace tinfra::path

#endif
