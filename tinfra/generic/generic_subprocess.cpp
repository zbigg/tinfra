#include <stdio.h>

#include "tinfra/subprocess.h"
#include "tinfra/tstring.h"
#include "tinfra/config.h"

#if defined(HAVE_POPEN)
// all clear
#elif defined(HAVE__POPEN)
FILE* popen(const char* command, const char* mode)
{
    return _popen(command, mode);
}
int pclose(FILE* pipe)
{
    return _pclose(pipe);
}
#else
#define NO_POPEN_SUBPROCESS
#endif

namespace tinfra {
namespace generic


#ifndef NO_POPEN_SUBPROCESS
struct subprocess_impl: public subprocess {
    stream* stdout_;
    stream* stdin_;
    
    FILE*  pipe_;
    int    exit_code_;
    
    subprocess_impl() :
        stdout_(0),
        stdin_(0),
        pipe_(0),
        exit_code_(-1)
    {
    }
    
    ~subprocess_impl() {
        delete stdin_;
        delete stdout_;
        wait();
    }
    
    virtual void     wait() {
        if( pipe_ == 0 ) 
            return;
        while( true ) {
            int exit_code_raw;
            int exit_code_raw = ::pclose(pipe_);
            
            if( exit_code_raw < 0 && errno == EINTR )
                continue;
	    pipe_ = 0;
            if( tpid < 0 )
                throw_io_exception("pclose");
            // now convert wait exit_code to process exit code as in wait(2)
            // manual
            if( WIFEXITED(exit_code_raw) ) {
                exit_code_ = WEXITSTATUS(exit_code_raw);
            } else {
                exit_code_ = EXIT_FAILURE;
            }            
            return;
        }
    }
    
    virtual int      get_exit_code() {        
        return exit_code_;
    }
    
    virtual void     terminate() {
        wait();
    }
    
    virtual void     kill() { 
        wait();
    }
    
    virtual stream*  get_stdin()  { return stdin_;  }
    virtual stream*  get_stdout() { return stdout_; }
    virtual stream*  get_stderr() { return 0; }
    
    virtual intptr_t get_native_handle() { return 0; }
    
    void start(const char* command) {
        do_start(command);
    }
    
    void     start(std::vector<std::string> const& args) {
        throw std::logic_error("generic::subprocess_impl: start(vector<>) not implemented");
    }
private:
    void do_start(tstring const& command)
    {
        string_pool temporary_string_pool;
        const bool fread =  (stdout_mode == REDIRECT);
        const bool fwrite = (stdin_mode == REDIRECT);
        // ignore stderr as it's only diagnostic output
        //const bool ferr  =  (stderr_mode == REDIRECT && !redirect_stderr);
        
        if( fread && fwrite )
            throw std::logic_error("generic::subprocess_impl: RW subprocess not supported on this platform");
        
        const char* open_mode = fread_ ? "rb" : "wb";
        
        pipe_ = ::popen(command.c_str(temporary_string_pool), open_mode)         
        if( pipe_ == 0 ) {
            throw_io_exception("popen");
        }
        
        throw std::logic_error("generic::subprocess_impl: FILE* based stream not implemented");
        // TODO:
        //stream* r = open_ansii(pipe_);
        stdout_ = stdin_ = 0;
        if( fread ) {
            stdout_ = r;
        } else {
            stdin_ = r;
        }
    }
};

} // end namespace generic

std::auto_ptr<subprocess> subprocess::create()
{
    return std::auto_ptr<subprocess>(new generic::subprocess_impl());
}

#else
/*
EVEN don't link such a program and give user choice what to do
std::auto_ptr<subprocess> subprocess::create()
{
    throw std::logic_error("generic::subprocess_impl: not supported on this platform");
}
*/
#endif

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

