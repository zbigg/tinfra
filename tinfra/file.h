#ifndef tinfra_file_h_included
#define tinfra_file_h_included

#include "stream.h"
#include "tstring.h"
#include "fs.h"

#include <inttypes.h>

namespace tinfra {

#ifdef FOM_READ
#error foo
#endif

enum file_open_flags {
    FOM_READ       = 0x01,
    FOM_WRITE      = 0x02,
    FOM_TRUNC      = 0x04,
    FOM_APPEND     = 0x08,
    FOM_CREATE     = 0x10,
    FOM_ATOMIC_CREATE = 0x20
};

typedef int file_output_flags;

class base_file: public input_stream, 
                 public output_stream {

public:
    enum seek_origin {
        SO_START,
        SO_END,
        SO_CURRENT
    };
    
    virtual ~base_file();

    virtual void close() = 0;
    
    virtual int seek(int pos, seek_origin origin = SO_START) = 0;    
    virtual tinfra::fs::file_info stat() = 0;
};

class file: public base_file {
public:
    typedef intptr_t handle_type;
    
    file(handle_type h);
    file(tstring const& name, int flags);
    ~file();
    
    static handle_type open_native(tstring const& name, int flags);
    
    // implementation of input_stream ...
    virtual void close();
    virtual int  read(char* dest, int size);

    // implementation of output_stream    
    virtual int write(const char* data, int size);    
    virtual void sync();
    
    // additional file specific functions
    virtual int seek(int pos, seek_origin origin = SO_START);
    
    virtual tinfra::fs::file_info stat();
    
    // move to native handle ??
    virtual intptr_t native() const;
    virtual void     release();

protected:    
    handle_type handle_;
};

/// read whole content of file
///
/// Read all data until EOF is occured and return
/// as std::string.
///
/// throws/failures:
///    will rethrow any error occurred in create_file_input_stream
///    will rethrow, any error occured in read(), 
///    will retrrow bad_alloc, from std::string
std::string read_file(tinfra::tstring name);

/// write whole buffer to file
///
/// Write all data (retrying as necessary) to file.
/// 
/// throws/failures:
///    will rethrow, any error occured in output.write()
///    std::runtime_error if output.write() returns 0
void        write_file(tinfra::tstring name, tstring const& data, file_output_flags = 0);


} // end of namespace tinfra

#endif // tinfra_file_h_included
