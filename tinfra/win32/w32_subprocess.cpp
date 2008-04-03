#include "tinfra/io/stream.h"
#include "tinfra/fmt.h"
#include "tinfra/win32.h"

#include <windows.h>

namespace tinfra {
    
template<> 
void holder<HANDLE>::release() {
    if( value_ != 0 ) {
        ::CloseJandle(value_);
        value_ = 0;
    }    
}

namespace win32 {
    
//
// pipe support for win32
//

struct win32_subprocess: public subprocess {
    win32_stream* stdin;
    win32_stream* stdout;
    win32_stream* stderr;
    
    HANDLE process_handle;
    
    int exit_code;
    
    win32_subprocess()
        : exit_code(-1)
    {
    }
    
    ~win32_subprocess() {
        delete stdin;
        delete stdout;
        if( stdin != stderr )
            delete stderr;
        
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
    
    virtual stream*  get_stdin()  { return stdin;  }
    virtual stream*  get_stdout() { return stdout; }
    virtual stream*  get_stderr() { return stderr; }
    
    virtual intptr_t get_native_handle() { return process_handle; }
    
    void start(const char* command) {
        holder<HANDLE> out_here(0);    // writing HERE -> child
        holder<HANDLE> out_remote(0);  // reading CHILD <- here
        
        holder<HANDLE> in_here(0);     // reading HERE <- child
        holder<HANDLE> in_emote(0);    // writing CHILD -> here
        
        holder<HANDLE> err_here(0);    // reading HERE <- child (diagnostic)
        holder<HANDLE> err_emote(0);   // writing CHILD -> here (diagnostic)
        
        const bool redirect_stderr = true;
        const bool fread =  (mode & std::ios::in) == std::ios::in;
        const bool fwrite = (mode & std::ios::out) == std::ios::out;
        const bool ferr  =  true && ! redirect_stderr;
        
        stdin = stderr = stdout = 0;
        
        if( fwrite ) {            
            if ( CreatePipe(&out_remote, &out_here, NULL, 4096) == 0 ) {
                throw_io_exception("CreatePipe failed");
            }
            SetHandleInformation(out[0],HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
        }
        if( fread ) {
            if ( CreatePipe(&in_here, &in_remote, NULL, 4096) == 0 ) {
                throw_io_exception("CreatePipe failed");
            }
            SetHandleInformation(in[1], HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
        }
        if( ferr ) {
            if ( CreatePipe(&err_here, &err_remote, NULL, 4096) == 0 ) {
                throw_io_exception("CreatePipe failed");
            }
            SetHandleInformation(err[1], HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
        }
        {	    /* spawn process */
            PROCESS_INFORMATION processi;
            STARTUPINFO si;
            DWORD creationFlags = 0;

            memset(&si,0,sizeof(si));
            si.cb = sizeof(si);
            /* GetStartupInfo(&si);*/
            
            if( fwrite ) {
                stdout = new win32_stream(out_here);
                out_here = 0;
                si.hStdInput = out_remote;
            } else {
                stdout = 0;
                si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
            }
            
            if( fread ) {
                stdin = new win32_stream(in_here);
                in_here = 0;
                si.hStdInput = in_remote;
            } else {
                stdin = 0;
                si.hStdInput = GetStdHandle(STD_OUTPUT_HANDLE);
            }
            
            if( redirect_stderr ) {
                stderr = stdin;
                hStdError = in_remote;
            } else if( ferr ) {
                stderr = new win32_stream(err_here);
                err_here = 0;
                hStdError = err_remote;
            } else {
                stderr = NULL;
                si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
            }
                        
            si.dwFlags |= STARTF_USESTDHANDLES;
            if( readf && writef ) {
                creationFlags |= DETACHED_PROCESS;
                si.dwFlags |= STARTF_USESHOWWINDOW;
                si.wShowWindow = SW_HIDE;
            }
            if( CreateProcess(
                NULL,
                (char*)program,
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
        }
    }
};

} // end namespace win32

//
// tinfra global stub for win32
//

subprocess* create_subprocess() {
    return new win32_subprocess();
}

} // end namespace tinfra::win32
