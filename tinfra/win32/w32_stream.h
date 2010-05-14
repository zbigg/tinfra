#ifndef tinfra_w32_stream_h_included
#define tinfra_w32_stream_h_included

#include "tinfra/stream.h"

#include "tinfra/win32.h"

namespace tinfra {
namespace win32 {

struct standard_handle_input: public tinfra::input_stream {
    standard_handle_input();
    ~standard_handle_input();

    void close();
    int read(char* dest, int size);
};

struct standard_handle_output: public tinfra::output_stream {
    standard_handle_output(bool is_err);
    ~standard_handle_output();

    using tinfra::output_stream::write;
    
    void close();
    int write(const char* data, int size);
    void sync();
private:
    bool is_err_;
};

} // end namespace tinfra::win32

extern win32::standard_handle_input  in;
extern win32::standard_handle_output out;
extern win32::standard_handle_output err;

} // end namespace tinfra

#endif // tinfra_w32_stream_h_included

