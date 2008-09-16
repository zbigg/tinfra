//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_os_common_h__
#define __tinfra_os_common_h__

namespace tinfra {
    

void throw_errno_error(int error, const std::string& message);
    
}

#endif

