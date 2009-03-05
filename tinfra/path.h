//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

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
bool is_executable(tstring const& name);

std::string basename(tstring const& name);

std::string dirname(tstring const& name);

std::string tmppath();

bool has_extension(tstring const& filename);
bool is_absolute(tstring const& filename);

std::string search_executable(tstring const& filename, tstring const& path);
std::string search_executable(tstring const& filename);

} } // end namespace tinfra::path

#endif

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++

