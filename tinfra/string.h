//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_string_h_included
#define tinfra_string_h_included

#include <string>
#include <vector>
#include "tinfra/tstring.h"

namespace tinfra {

    
void        escape_c_inplace(std::string& a);
std::string escape_c(tstring const& a);
    
void        strip_inplace(std::string& s);
std::string strip(tstring const& s);

void        chop_inplace(std::string& s);
std::string chop(tstring const& s);

/// strict string splitter
///
/// repeated sibling delimiters create new substrings
std::vector<std::string> split_strict(tstring const& in, char delimiter);

/// strict string splitter
///
/// repeated sibling act as one delimiter
std::vector<std::string> split_skip_empty(tstring const& in, char delimiter);

/// strict splitting
///
/// same as split_strictt 
/// DEPRECATED
std::vector<std::string> split(tstring const& in, char delimiter); // DEPRECATED!

/// line spliting
///
/// intelligent strict line splitting
/// removes end-of-line characters
/// detects only following line endings CRLF and LF 
/// (CR alone is not detected!)
std::vector<std::string> split_lines(tstring const& in);

/// Compare strings ignoring case.
///
/// Compare strings ignoring case (strnicmp, strncasecmp) functions are used
/// or std::tolower fallback if former are not availabled.
///
/// If strings are equal up to common length then longer string is considered
/// "bigger".
///
/// @return less than 0 when a appears to be "less" than b, 0 if they're equal, greater than 0 otherwise
int         compare_no_case(tstring const& a, tstring const& b);

/// Compare strings ignoring case.
///
/// Compare strings ignoring case (strnicmp, strncasecmp) functions are used
/// or std::tolower fallback if former are not availabled.
///
/// If strings are equal up to <upto> paramerer then 0 is returned and
/// as they would be equal.
///
/// @return less than 0 when a appears to be "less" than b, 0 if they're equal, greater than 0 otherwise
int         compare_no_case(tstring const& a, tstring const& b, size_t upto);

}

#endif
