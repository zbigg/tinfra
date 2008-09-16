//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_path__
#define __tinfra_path__

#include <string>

namespace tinfra { 
namespace path {
    
std::string join(const std::string& a, const std::string& b);
    
bool exists(const char* name);  // TODO: move to fs
    
inline 
bool exists(const std::string& name) { return exists(name.c_str()); }


bool is_file(const char* name); // TODO: move to fs
bool is_dir(const char* name);  // TODO: move to fs

inline
bool is_dir(std::string const& name) { return is_dir(name.c_str()); }  // TODO: move to fs

std::string basename(const std::string& name);
inline std::string basename(const char* name) { return basename(std::string(name)); }

std::string dirname(const std::string& name);
inline std::string dirname(const char* name) { return dirname(std::string(name)); }

std::string tmppath(const char* prefix = 0 , const char* tmpdir = 0);

} } // end namespace tinfra::path

#endif
