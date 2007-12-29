#include "tinfra/fmt.h"
#include "tinfra/exception.h"
#include <string>

namespace fp {
    
} // end namespace fp

std::ostream& operator << (std::ostream& out, tinfra::simple_fmt& fmt)
{
    out << fmt.str();
}
