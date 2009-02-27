//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "tinfra/platform.h"

#include <stdexcept>
#include <cstring>
#include <errno.h>

#include "tinfra/io/stream.h"
#include "tinfra/fmt.h"

namespace tinfra {
    
template <typename E>
static void throw_errno(int error, const std::string& message)
{
    std::string error_str = ::strerror(error);
    std::string full_message = tinfra::fmt("%s: %s") % message % error_str;
    throw E(full_message);
}

void throw_errno_error(int error, std::string const & message)
{
    switch(error) {
    case ENAMETOOLONG:
    case EBADF:
    case E2BIG:
    case EINVAL:
        throw_errno<std::invalid_argument>(error, message);
    
    case EFBIG:
    case ENOSPC:
        throw_errno<std::length_error>(error, message);
    
    case EAGAIN:
        throw_errno<tinfra::io::would_block>(error, message);
    
    case ENOENT:
    case EISDIR:
    case ENOTDIR: 
        throw_errno<std::logic_error>(error, message);
    
    default:
        throw_errno<std::runtime_error>(error, message);
    }
}

} // end namespace tinfra
