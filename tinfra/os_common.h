//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef tinfra_os_common_h_included
#define tinfra_os_common_h_included

namespace tinfra {
    

void throw_errno_error(int error, const std::string& message);
    
}

#endif

