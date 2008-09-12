#ifndef __tinfra_os_common_h__
#define __tinfra_os_common_h__

namespace tinfra {
    

void throw_errno_error(int error, const std::string& message);
    
}

#endif

