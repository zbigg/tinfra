#ifndef __tinfra_win32_h__
#define __tinfra_win32_h__

namespace tinfra { namespace win32 {

std::string get_error_string(unsigned int error_code);

void throw_system_error(unsigned int error_code, std::string const& message);
void throw_system_error(std::string const& message);

} } // end namespace tinfra::win32

#endif
