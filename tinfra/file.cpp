#include "file.h" // we implement this

#include "stream.h" // for ...
#include "buffered_stream.h"

#include <memory>

namespace tinfra {

using std::auto_ptr;

base_file::~base_file()
{
}


//static 
file::handle_type file::open_native(tstring const& name, int flags)
{
    file f(name, flags);
    handle_type h = f.native();
    f.release();
    
    return h;
}

auto_ptr<input_stream> create_file_input_stream(tstring const& name, size_t buffer_size)
{
    tinfra::file* delegate = new file(name, FOM_READ);
    return auto_ptr<input_stream>(new owning_buffered_input_stream(delegate, buffer_size));
}

auto_ptr<input_stream> create_file_input_stream(tstring const& name)
{
    return auto_ptr<input_stream>(new file(name, FOM_READ));
}

auto_ptr<output_stream> create_file_output_stream(tstring const& name, int mode)
{
    if( mode == 0 ) {
        mode = FOM_TRUNC | FOM_CREATE;
        
    }
    mode |= FOM_WRITE;
    
    return auto_ptr<output_stream>(new file(name, mode));
}

std::string read_file(tinfra::tstring name)
{
    file fin(name, FOM_READ);
    
    return read_all(fin);
}

void        write_file(tinfra::tstring name, tstring const& data, int mode)
{
    if( mode == 0 ) {
        mode = FOM_TRUNC | FOM_CREATE;
    }
    mode |= FOM_WRITE;
    file fout(name, mode);
    
    write_all(fout, data);
    
}


} // end of namespace tinfra
