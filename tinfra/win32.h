//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_win32_h__
#define __tinfra_win32_h__

namespace tinfra { namespace win32 {

std::string get_error_string(unsigned int error_code);

void throw_system_error(unsigned int error_code, std::string const& message);
void throw_system_error(std::string const& message);

#define TINFRA_LOG_WIN32_ERROR(msg,le) \
    TINFRA_LOG_ERROR(tinfra::fmt("%s: %s (%i)") % (msg) % tinfra::win32::get_error_string(le) % (le));

} } // end namespace tinfra::win32

#endif
