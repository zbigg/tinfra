#ifndef __tinfra_string_h__
#define __tinfra_string_h__

#include <string>
#include <vector>

namespace tinfra {

void        escape_c_inplace(std::string& a);
std::string escape_c(const std::string& a);
    
void        strip_inplace(std::string& s);
std::string strip(const std::string& s);

void        chop_inplace(std::string& s);
std::string chop(std::string const& s);

std::vector<std::string> split(std::string const& in, const char* delimiters);
std::vector<std::string> split_lines(std::string const& in);
    
}

#endif
