
#include <sys/types.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>

#include "tinfra/io/stream.h"
#include "tinfra/fmt.h"
#include "tinfra/subprocess.h"

namespace tinfra {

namespace posix {

using tinfra::io::stream;
using tinfra::io::io_exception;
using tinfra::io::open_native;
    
enum {
    MAX_NUMBER_OF_FILES = NOFILE // defined in <sys/param.h> on linux 
};

class fd_holder {
    int fd;
public:
    explicit fd_holder(int fd): fd(fd) {}
        
    ~fd_holder() { 
        if( fd != -1 )
            ::close(fd);
    }
    
    fd_holder& operator =(int fd) { this->fd = fd; return *this; }
    
    operator int () const { return fd; }
};

static void throw_io_exception(const char* message)
{
    throw io_exception(fmt("%s: %s") % message % strerror(errno));
}

static int execute_command(const char* command)
{
    int r = system(command);
    if( r < 0 )
        throw_io_exception("system failed");
    if( WIFEXITED(r) ) {
        r = WEXITSTATUS(r);
    } else {
        r = EXIT_FAILURE;
    }
    return r;
}

static int execute_command(std::vector<std::string> const& args)
{
    char** raw_args;
    raw_args = new char*[args.size()+1];
    {
        int idx = 0;
        for(std::vector<std::string>::const_iterator i=args.begin(); i != args.end(); ++i ) {
            raw_args[idx++] = const_cast<char*>(i->c_str());
        }
        raw_args[idx] = 0;
    }
    execvp(raw_args[0], raw_args);
    throw_io_exception("exec failed");
    return 0;
}
//
// subprocess support for posix
//

struct posix_subprocess: public subprocess {
    stream* stdin;
    stream* stdout;
    stream* stderr;
    
    int pid;
    
    int exit_code;
    
    posix_subprocess() : 
        pid(-1),
        exit_code(-1)
    {
    }
    
    ~posix_subprocess() {
        delete stdin;
        delete stdout;
        if( stdin != stderr )
            delete stderr;
    }
    
    virtual void     wait() {
        if( pid == -1 ) 
            return;
        while( true ) {
            int exit_code_raw;
            int tpid = ::waitpid(pid, &exit_code_raw, 0);
            
            if( tpid < 0 &&  errno == EINTR )
                continue;
            if( tpid < 0 )
                throw_io_exception("waitpid");
            // now convert wait exit_code to process exit code as in wait(2)
            // manual
            if( WIFEXITED(exit_code_raw) ) {
                exit_code = WEXITSTATUS(exit_code_raw);
            } else {
                exit_code = EXIT_FAILURE;
            }
            //std::cerr << "PSP: waitpid(" << pid << ") returned " << exit_code << std::endl;            
            return;
        }
    }
    
    virtual int      get_exit_code() {        
        return exit_code;
    }
    
    virtual void     terminate() {
        if ( ::kill(pid, SIGTERM) < 0 ) 
            throw_io_exception(fmt("kill(%i,SIGTERM)") % pid);
    }
    
    virtual void     kill() { 
        if ( ::kill(pid, SIGKILL) < 0 ) 
            throw_io_exception(fmt("kill(%i,SIGKILL)") % pid);
    }
    
    virtual stream*  get_stdin()  { return stdin;  }
    virtual stream*  get_stdout() { return stdout; }
    virtual stream*  get_stderr() { return stderr; }
    
    virtual intptr_t get_native_handle() { return pid; }
    
    void start(const char* command) {
        do_start(command);
    }
    
    void     start(std::vector<std::string> const& args) {
        do_start(args);
    }
    
    template <typename T>
    void do_start(T const& command)
    {
        fd_holder out_here(-1);    // writing HERE -> child
        fd_holder out_remote(-1);  // reading CHILD <- here
        
        fd_holder in_here(-1);     // reading HERE <- child
        fd_holder in_remote(-1);   // writing CHILD -> here
        
        fd_holder err_here(-1);    // reading HERE <- child (diagnostic)
        fd_holder err_remote(-1);  // writing CHILD -> here (diagnostic)
                
        const bool fread =  (stdout_mode == REDIRECT);
        const bool fwrite = (stdin_mode == REDIRECT);
        const bool ferr  =  (stderr_mode == REDIRECT && !redirect_stderr);
        
        stdin = stderr = stdout = 0;
        
        if( fwrite ) {            
            int boo[2];
            if ( pipe(boo) < 0 ) 
                throw_io_exception("pipe");
            out_remote = boo[0];
            out_here = boo[1];
        }
        if( fread ) {
            int boo[2];
            if ( pipe(boo) < 0 ) 
                throw_io_exception("pipe");
            
            in_here   = boo[0];
            in_remote = boo[1];
        }
        if( ferr ) {
            int boo[2];
            if ( pipe(boo) < 0 ) 
                throw_io_exception("pipe");
            
            err_here   = boo[0];
            err_remote = boo[1];
        }
        
        // TODO: it should rather be vfork
        pid = ::fork();
        if( pid == -1 ) 
            throw_io_exception("fork");
            
        if( pid == 0 ) {
            // children part            
            if( fwrite ) {
                int a = ::dup(out_remote);
                ::dup2(a,0);
                ::close(a);
            } else if( stdin_mode == NONE) { 
                ::close(0);
                ::open("/dev/null",O_RDONLY);
            }
            if( fread ) {
                int a = ::dup(in_remote);
                ::dup2(a,1);
                ::close(a);
            } else if( stdout_mode == NONE) { 
                ::close(1);
                ::open("/dev/null",O_WRONLY);
            }
            
            if( ferr ) {
                int a = ::dup(err_remote);
                ::dup2(a,2);
                ::close(a);
            } else if (redirect_stderr ) {
                ::dup2(1, 2);
            } else if( stderr_mode == NONE) { 
                ::close(2);
                ::open("/dev/null",O_WRONLY);
            }

            for( int fd = 3; fd < MAX_NUMBER_OF_FILES; ++fd ) 
                ::close(fd);
            
            if( ::chdir(working_dir.c_str()) < 0 ) {
                std::cerr << "TIC: unable to change working dir!" << std::endl;
                ::exit(127);
            }
            // TODO it should rather be exec
            int result = execute_command(command);
            //std::cerr << "PSP: execute_command() returned " << result << std::endl;
            if( result < 0 ) {
                std::cerr << "TIC: system() failed!" << std::endl;
            }
            _exit(result);
        } else {
            if( fwrite ) {
                stdin = open_native(out_here);
                out_here = -1;
            }
            
            if( fread ) {
                stdout = open_native(in_here);
                in_here = -1;
            }
            
            if( ferr ) {
                stderr = open_native(err_here);
                err_here = -1;
            } else if( redirect_stderr ) {
                stderr = stdout;
            } 
        }
    }
};

} // end namespace posix

//
// tinfra global stub for posix
//

subprocess* create_subprocess() {
    return new posix::posix_subprocess();
}

} // end namespace tinfra
