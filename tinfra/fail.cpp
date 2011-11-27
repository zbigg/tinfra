#include "fail.h"
#include "logger.h"
#include <string>

namespace tinfra {

//
// fail
//

void fail(tstring const& m, tstring const& cause)
{
    tinfra::failure exception(m, cause);
    tinfra::log_error(exception.what());
    throw exception;
}

void fail(tstring const& m, tstring const& cause, tinfra::trace::location const& loc)
{
    tinfra::failure exception(m, cause);
    tinfra::log_error(exception.what(), loc);
    throw exception;
}

//
// failure 
//
failure::failure(tstring const& message): 
    std::runtime_error(message) 
{
}

failure::failure(tstring const& message, tstring const& cause): 
    std::runtime_error(std::string(message) + ": " + cause.str()) 
{
}

} // end namespace tinfra

