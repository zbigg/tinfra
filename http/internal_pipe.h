#ifndef tinfra_internal_pipe_h
#define tinfra_internal_pipe_h
    
#include <tinfra/stream.h> // for tinfra::(input|output)_stream
#include <memory> // for auto_ptr

namespace tinfra {

class internal_pipe: public tinfra::input_stream, 
                     public tinfra::output_stream
{
    enum {
        UNLIMITED = 0
    };
public:
    internal_pipe(int buffer_size);
    
    /// Blocking read from pipe.
    ///
    /// This method may block if pipe internal buffer is empty i.e there is 
    /// no data to read at given time.
    /// 
    /// On success, it return number of bytes readed (may be less than requested size)
    /// On EOF it returns 0
    /// On internal error it throw std::runtime_error with apropriate description.
    int read(char* dest, int size);

    /// Blocking write to pipe.
    ///
    /// This function may block if pipe internal buffer is full.
    /// output_stream interface
    ///
    /// On success, it return number of bytes written (may be less than requested size)
    /// On EOF it returns 0
    /// On internal error it throw std::runtime_error with apropriate description
    /// In case of error during write, after some bytes were successfully commited to buffer, function 
    /// returns succeeds and return of successfully delivered bytes. 
    /// Following write operation will signal this failure with exception.
    ///
    /// May throw bad_alloc if buffer allocation fails.
    int write(const char* data, int size);
    
    /// Synchronize buffers with underlying storage.
    ///
    /// Noop.
    void sync();
    
    /// Close pipe.
    ///
    /// Mark that no more data will be written to this pipe (This flag actually marks an EOF)
    /// flag inside a pipe.
    /// The behaviour of write and read calls with EOF=true is following;
    ///  - write is impossible and throws XXX tbd.
    ///  - read will return all data that has been commited to buffer before close
    ///    and when buffer is empty will return EOF(0)
    /// Throws nothing.
    void close();
private:
    // noncopyable
    internal_pipe(internal_pipe const&);
    internal_pipe& operator=(internal_pipe const&);

    class implementation_detail;
    std::auto_ptr<implementation_detail> impl;
};    
} // end namespace tinfra

#endif // tinfra_internal_pipe_h
