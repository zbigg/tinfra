#ifndef __tinfra_io_file_h__
#define __tinfra_io_file_h__

#include <ios>

namespace tinfra {
namespace io {

class stream;
    
namespace file {

stream* open_native(void* handle);
stream* open_file(const char* name, std::ios::openmode mode);
    
}}}

#endif
