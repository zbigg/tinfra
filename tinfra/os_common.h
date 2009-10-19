//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_os_common_h_included
#define tinfra_os_common_h_included

namespace tinfra {
    

void throw_errno_error(int error, const std::string& message);
    
}

#endif

