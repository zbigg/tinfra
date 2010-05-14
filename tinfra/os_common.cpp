//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
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
    case EINVAL:
        throw_errno<std::invalid_argument>(error, message);
    
    case EAGAIN:
        throw_errno<tinfra::io::would_block>(error, message);
    
    case E2BIG:
    case ENOSPC:
    case ENOENT:
    case EISDIR:
    case ENOTDIR: 
    default:
        throw_errno<std::runtime_error>(error, message);
    }
}

} // end namespace tinfra
