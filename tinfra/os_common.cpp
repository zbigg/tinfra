#include <stdexcept>
#include <cstring>
#include <errno.h>

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
    
    case ENOENT:
    case EISDIR:
    case ENOTDIR: 
        throw_errno<std::logic_error>(error, message);
    
    default:
        throw_errno<std::runtime_error>(error, message);
    }
}

} // end namespace tinfra
