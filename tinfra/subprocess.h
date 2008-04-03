#ifndef __tinfra_subprocess_h__
#define __tinfra_subprocess_h__

#include "tinfra/io/stream.h"

namespace tinfra {

class subprocess {
public:
    enum pipe_mode {
        INHERIT,
        REDIRECT, 
        NONE
    };
    subprocess() : 
        stdin_mode(INHERIT), 
        stdout_mode(INHERIT), 
        stderr_mode(INHERIT), 
        redirect_stderr(false), 
        working_dir(".")
    {
    }
    void set_working_dir(std::string const& dir ) { working_dir = dir; }
    // TODO: add environment setting
    
    void set_redirect_stderr(bool r = true) { redirect_stderr = r; }
    
    void set_stdin_mode(pipe_mode pm)   { stdin_mode = pm; }
    void set_stdout_mode(pipe_mode pm)  { stdout_mode = pm; }
    void set_stderr_mode(pipe_mode pm)  { stderr_mode = pm; }
    
    virtual ~subprocess();
    
    virtual void     start(const char* command) = 0;
    
    virtual void     wait() = 0;
    virtual int      get_exit_code() = 0;
    
    virtual void     terminate() = 0;
    virtual void     kill() = 0;
    
    virtual tinfra::io::stream*  get_stdin() = 0;
    virtual tinfra::io::stream*  get_stdout() = 0;
    virtual tinfra::io::stream*  get_stderr() = 0;
    
    virtual intptr_t get_native_handle() = 0;
    
protected:
    pipe_mode stdin_mode;
    pipe_mode stdout_mode;
    pipe_mode stderr_mode;

    bool redirect_stderr;

    std::string working_dir;
};

}

#endif // #ifndef __tinfra_subprocess_h__
