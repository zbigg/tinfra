//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_stream_h_included
#define tinfra_stream_h_included

#include "tstring.h" // for tstring
#include <memory>  // for auto_ptr

namespace tinfra {

class input_stream {
public:
    virtual ~input_stream();

    virtual void close() = 0;

    virtual int read(char* dest, int size) = 0;
};

class output_stream {
public:
    virtual ~output_stream();

    // common interface
    int write(tstring const& data);

    // abstract interface
    virtual void close() = 0;    
    virtual int write(const char* data, int size) = 0;
    virtual void sync() = 0;
};

#if 0
// adaptable & others: we don't have those yet ready
// TODO: this is only idea of adding "adaptable"
//       pattern to tinfra::io
//       channels would have get_adaptable<T>
//       and return proper interface entry if needed
class position_controller {
    virtual ~position_controller();
    
    enum seek_origin {
        start,
        end,
        current
    };
    
    virtual int seek(int pos, seek_origin origin = start) = 0;
};
#endif

std::auto_ptr<input_stream>  create_file_input_stream(tstring const& name);
std::auto_ptr<input_stream>  create_file_input_stream(tstring const& name, size_t buffer_size);

/*
    to be rethinked!
    
class file_input_stream: public input_stream
{
    file_input_stream(tstring const& name);
    ~file_input_stream();
private:
    // noncopyable
    file_input_stream(file_input_stream const&);
    file_input_stream& operator=(file_input_stream const&);
    
    std::auto_ptr<input_stream> impl;
};
*/
enum memory_strategy {
    USE_BUFFER,
    COPY_BUFFER
};

std::auto_ptr<input_stream>  create_memory_input_stream(const void* buffer, size_t size, memory_strategy buffer_copy);
std::auto_ptr<output_stream> create_memory_output_stream(std::string& out);


std::auto_ptr<output_stream> create_file_output_stream(tstring const& name, int mode);

/// read whole content of stream
///
/// Read all data until EOF is occured and return
/// as std::string.
///
/// throws/failures:
///    will rethrow, any error occured in output.read(), 
///    will retrrow bad_alloc, from std::string    
std::string read_all(input_stream& input);

/// write whole buffer to stream
///
/// Write all data (retrying as necessary) to target stream.
/// 
/// throws/failures:
///    will rethrow, any error occured in output.write()
///    std::runtime_error if output.write() returns 0
void        write_all(output_stream& output, tstring const& data);

void        stream_copy(input_stream& input, output_stream& out);

} // end namespace tinfra

// native_file is defined "per"
// platform

#if   defined( _WIN32)
#include "win32/w32_stream.h"
#else
#include "posix/posix_stream.h"
#endif // disabling of platform specific defs

#endif // tinfra_stream_h_included

