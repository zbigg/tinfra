//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_win32_h_included
#define tinfra_win32_h_included

#include "tstring.h"
#include "logger.h"

#include <vector>
#include <string>

namespace tinfra { namespace win32 {

std::string get_error_string(unsigned int error_code);

/// Throw exception for system error.
///
/// Windows system error code is formatted as message, concated with parameter
/// and appropriate std::exception is thrown.
void throw_system_error(unsigned int error_code, std::string const& message);

/// Throw exception for last system error.
///
/// Last windows system error code (GetLastError) is formatted as message, concated 
/// with parameter
/// and appropriate std::exception is thrown.
void throw_system_error(std::string const& message);
    
void get_available_drives(std::vector<std::string>& result);

std::wstring make_wstring_from_utf8(tstring const& str);
std::wstring make_wstring_from(tstring const& str, int encoding);
std::string make_utf8(wchar_t const* str);

#define TINFRA_LOG_WIN32_ERROR(msg,le) \
    TINFRA_LOG_ERROR(tinfra::fmt("%s: %s (%i)") % (msg) % tinfra::win32::get_error_string(le) % (le));

} } // end namespace tinfra::win32

#endif // tinfra_win32_h_included
