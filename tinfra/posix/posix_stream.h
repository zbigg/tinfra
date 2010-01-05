#ifndef tinfra_posix_stream_h_included
#define tinfra_posix_stream_h_included

#include "tinfra/stream.h"

namespace tinfra {
namespace posix {

struct standard_handle_input: public tinfra::input_stream {
    standard_handle_input();
    ~standard_handle_input();

    void close();
    int read(char* dest, int size);
};

struct standard_handle_output: public tinfra::output_stream {
    standard_handle_output(int where);
    ~standard_handle_output();

    using tinfra::output_stream::write;
    
    void close();
    int write(const char* data, int size);
    void sync();
private:
    bool is_err_;
};

} // end namespace tinfra::posix

extern posix::standard_handle_input  in;
extern posix::standard_handle_output out;
extern posix::standard_handle_output err;

} // end namespace tinfra

#endif // tinfra_posix_stream_h_included

