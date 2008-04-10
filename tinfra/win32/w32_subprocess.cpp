#include "tinfra/subprocess.h"

#include "tinfra/io/stream.h"
#include "tinfra/fmt.h"
#include "tinfra/win32.h"
#include "tinfra/holder.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace tinfra {
    
template<> 
void holder<HANDLE>::release() {
    if( value_ != 0 ) {
        ::CloseHandle(value_);
        value_ = 0;
    }    
}

namespace win32 {

using tinfra::io::stream;
using tinfra::io::io_exception;
using tinfra::io::open_native;

static void throw_io_exception(const char* message)
{
    unsigned int error = ::GetLastError();
    throw io_exception(fmt("%s: %s(%i)") % message % get_error_string(error) % error);
}

//
// pipe support for win32
//

struct win32_subprocess: public subprocess {
    stream* stdin_;
    stream* stdout_;
    stream* stderr_;
    
    HANDLE process_handle;
    
    int exit_code;
    
    win32_subprocess()
        : exit_code(-1)
    {
    }
    
    ~win32_subprocess() {
        delete stdin_;
        delete stdout_;
        if( stdout_ != stderr_ )
            delete stderr_;
        
        get_exit_code();
        
        if( !::CloseHandle(process_handle) ) {
            // TODO: silent win32 error
        }
    }
    
    virtual void     wait() {
        if( WaitForSingleObject(process_handle, INFINITE) != WAIT_OBJECT_0 ) {
            // TODO: win32_error
        }
    }
    
    virtual int      get_exit_code() {
        if( process_handle != NULL ) {
            DWORD dwExitCode;
            if( !::GetExitCodeProcess(process_handle,&dwExitCode) ) {
                // TODO: win32_error
            }
            exit_code = dwExitCode;
        }
        return exit_code;
    }
    
    virtual void     terminate() {
        // unfortunately there is no way to grcefully kill 
        // win32 process
        kill();
    }
    virtual void     kill() { 
        if( !::TerminateProcess(process_handle, 10) ) {
            // TODO: win32_error
        }
    }
    
    virtual stream*  get_stdin()  { return stdin_;  }
    virtual stream*  get_stdout() { return stdout_; }
    virtual stream*  get_stderr() { return stderr_; }
    
    virtual intptr_t get_native_handle() { return reinterpret_cast<intptr_t>(process_handle); }
    
    void start(const char* command) {
        /*
        holder<HANDLE> out_here(0);     // writing HERE -> child
        holder<HANDLE> out_remote(0);   // reading CHILD <- here
        
        holder<HANDLE> in_here(0);      // reading HERE <- child
        holder<HANDLE> in_remote(0);    // writing CHILD -> here
        
        holder<HANDLE> err_here(0);     // reading HERE <- child (diagnostic)
        holder<HANDLE> err_remote(0);   // writing CHILD -> here (diagnostic)
        */
        HANDLE out_here(0);     // writing HERE -> child
        HANDLE out_remote(0);   // reading CHILD <- here
        
        HANDLE in_here(0);      // reading HERE <- child
        HANDLE in_remote(0);    // writing CHILD -> here
        
        HANDLE err_here(0);     // reading HERE <- child (diagnostic)
        HANDLE err_remote(0);   // writing CHILD -> here (diagnostic)
        
        const bool fread =  (stdout_mode == REDIRECT);
        const bool fwrite = (stdin_mode  == REDIRECT);
        const bool ferr  =  (stderr_mode == REDIRECT && !redirect_stderr);
        
        stdin_ = stderr_ = stdout_ = 0;
        
        {
            SECURITY_ATTRIBUTES saAttr; 
            
            saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
            saAttr.bInheritHandle = TRUE; 
            saAttr.lpSecurityDescriptor = NULL; 
            
            if( fwrite ) {            
                if ( CreatePipe(&out_remote, &out_here, &saAttr, 0) == 0 ) {
                    throw_io_exception("CreatePipe failed");
                }
                SetHandleInformation(out_remote, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
                SetHandleInformation(out_here,   HANDLE_FLAG_INHERIT, 0);
            }
            if( fread ) {
                if ( CreatePipe(&in_here, &in_remote, &saAttr, 0) == 0 ) {
                    throw_io_exception("CreatePipe failed");
                }
                SetHandleInformation(in_remote, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
                SetHandleInformation(in_here,   HANDLE_FLAG_INHERIT, 0);
            }
            if( ferr ) {
                if ( CreatePipe(&err_here, &    err_remote, &saAttr, 0) == 0 ) {
                    throw_io_exception("CreatePipe failed");
                }
                SetHandleInformation(err_remote, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
                SetHandleInformation(err_here,   HANDLE_FLAG_INHERIT, 0);
            }
        }
        {	    /* spawn process */
            STARTUPINFO si;
            
            PROCESS_INFORMATION processi;
            DWORD creationFlags = 0;

            memset(&processi,0,sizeof(processi));
            
            memset(&si,0,sizeof(si));
            si.cb = sizeof(si);
            /* GetStartupInfo(&si);*/
            
            if( fwrite ) {
                stdin_ = open_native(reinterpret_cast<intptr_t>((HANDLE)out_here));
                out_here = 0;
                si.hStdInput = out_remote;
            } else {
                stdin_ = 0;
                si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
            }
            
            if( fread ) {
                stdout_ = open_native(reinterpret_cast<intptr_t>((HANDLE)in_here));
                in_here = 0;
                si.hStdOutput = in_remote;
            } else {
                stdout_ = 0;
                si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
            }
            
            if( redirect_stderr && fread ) {
                stderr_ = stdout_;
                si.hStdError = in_remote;
            } else if( ferr ) {
                stderr_ = open_native(reinterpret_cast<intptr_t>((HANDLE)err_here));
                err_here = 0;
                si.hStdError = err_remote;
            } else {
                stderr_ = NULL;
                si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
            }
                        
            si.dwFlags |= STARTF_USESTDHANDLES;
            
            //creationFlags |= DETACHED_PROCESS;
            //si.dwFlags |= STARTF_USESHOWWINDOW;
            //si.wShowWindow = SW_HIDE;
        
            if( CreateProcess(
                NULL,
                (char*)command,
                NULL,
                NULL,
                TRUE,
                creationFlags,
                NULL,
                NULL,
                &si,
                &processi) == 0 )
            {
                throw_io_exception("CreateProcess failed");
            }
            process_handle = processi.hProcess;
            if( in_remote )
                ::CloseHandle(in_remote);
            if( out_remote )
                ::CloseHandle(out_remote);
            if( err_remote )
                ::CloseHandle(err_remote);
        }
    }
};

} // end namespace win32

//
// tinfra global stub for win32
//

subprocess* create_subprocess() {
    return new win32::win32_subprocess();
}

} // end namespace tinfra::win32
