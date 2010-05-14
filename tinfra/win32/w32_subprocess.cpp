//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/platform.h"

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

static HANDLE open_null_for_subprocess(std::ios::openmode mode)
{
    stream* stream = tinfra::io::open_file("NUL", mode);
    HANDLE result = reinterpret_cast<HANDLE>(stream->native());
    stream->release();
    delete stream;
    SetHandleInformation(result, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    return result;
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
    environment_t env;
    bool env_set;
    
    win32_subprocess() : 
        process_handle(0), 
        exit_code(-1),
        env_set(false)
    {
    }
    
    ~win32_subprocess() {
        delete stdin_;
        delete stdout_;
        if( stdout_ != stderr_ )
            delete stderr_;
        try {
            get_exit_code();
        } catch(std::exception&) {
            // TODO: silent win32 error 
        }
        if( process_handle ) {
            if( !::CloseHandle(process_handle) ) {
                // TODO: silent win32 error
            }
        }
    }
    
    virtual void     wait() {
        if( WaitForSingleObject(process_handle, INFINITE) != WAIT_OBJECT_0 ) {
            throw_system_error("WaitForSingleObject on subprocess failed");
        }
    }

    virtual void     detach() {
        if( process_handle ) {
            if( !::CloseHandle(process_handle) ) {
                // TODO: silent win32 error
            }
            process_handle = 0;
        }
        exit_code = -1;
    }
    
    virtual int      get_exit_code() {
        if( exit_code == -1 && process_handle != NULL ) {
            DWORD dwExitCode;
            if( !::GetExitCodeProcess(process_handle,&dwExitCode) ) {
                throw_system_error("GetExitCodeProcess failed");
            }
            if( dwExitCode == STILL_ACTIVE )
                return -1;
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
            throw_system_error("TerminateProcess failed");
        }
    }
    
    virtual stream*  get_stdin()  { return stdin_;  }
    virtual stream*  get_stdout() { return stdout_; }
    virtual stream*  get_stderr() { return stderr_; }
    
    virtual intptr_t get_native_handle() { return reinterpret_cast<intptr_t>(process_handle); }

    virtual void     set_environment(environment_t const& e)
    {
        env = e;
        env_set = true;
    }
    void start(std::vector<std::string> const& args) {
        std::ostringstream cmd;
        for(std::vector<std::string>::const_iterator i = args.begin(); i != args.end(); ++i ) {
            if( i != args.begin() ) 
                cmd << " ";
            if( i->find_first_of(" \t") )
                cmd << "\"" << *i << "\"";
            else
                cmd << *i;
        }
        start(cmd.str().c_str());
    }
    
    void start(const char* command) {
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
                    throw_system_error("CreatePipe failed");
                }
                SetHandleInformation(out_remote, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
                SetHandleInformation(out_here,   HANDLE_FLAG_INHERIT, 0);
            }
            if( fread ) {
                if ( CreatePipe(&in_here, &in_remote, &saAttr, 0) == 0 ) {
                    throw_system_error("CreatePipe failed");
                }
                SetHandleInformation(in_remote, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
                SetHandleInformation(in_here,   HANDLE_FLAG_INHERIT, 0);
            }
            if( ferr ) {
                if ( CreatePipe(&err_here, &    err_remote, &saAttr, 0) == 0 ) {
                    throw_system_error("CreatePipe failed");
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
            } else if( stdin_mode == NONE) {
                si.hStdInput = out_remote = open_null_for_subprocess(std::ios::in);                
            } else {
                si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
            }
            
            if( fread ) {
                stdout_ = open_native(reinterpret_cast<intptr_t>((HANDLE)in_here));
                in_here = 0;
                si.hStdOutput = in_remote;
            } else if( stdout_mode == NONE) {
                si.hStdOutput = in_remote = open_null_for_subprocess(std::ios::out);
            } else {
                si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
            }
            
            if( redirect_stderr && fread ) {
                stderr_ = stdout_;
                si.hStdError = in_remote;
            } else if( ferr ) {
                stderr_ = open_native(reinterpret_cast<intptr_t>((HANDLE)err_here));
                err_here = 0;
                si.hStdError = err_remote;
            } else if( stderr_mode == NONE ) {
                si.hStdError = err_remote = open_null_for_subprocess(std::ios::out);
            } else{
                stderr_ = NULL;
                si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
            }
                        
            si.dwFlags |= STARTF_USESTDHANDLES;
            
            // TODO: decide what to do
            // When DETACHED_PROCESS is in use child doesn't inherit console handles
            // even if they were explicitly given in STARTUPINFO.hStdXXX.            
            //creationFlags |= DETACHED_PROCESS;
            si.dwFlags |= STARTF_USESHOWWINDOW;
            si.wShowWindow = SW_HIDE;
            std::vector<char> environment_bytes;
            if( env_set )
                make_environment_block(environment_bytes);
            int ret = CreateProcess(
                NULL,
                (char*)command,
                NULL,
                NULL,
                TRUE,
                creationFlags,
                (env_set ? &(environment_bytes.at(0)) : NULL),
                NULL,
                &si,
                &processi);
            
            if( in_remote )
                ::CloseHandle(in_remote);
            if( out_remote )
                ::CloseHandle(out_remote);
            if( err_remote )
                ::CloseHandle(err_remote);
            
            if( ret == 0 )
            {
                throw_system_error("CreateProcess failed");
            }
            process_handle = processi.hProcess;
        }
    }
    
    void make_environment_block(std::vector<char>& result) const
    {
        {
            size_t env_size = 1;
            for( environment_t::const_iterator i = env.begin(); i != env.end(); ++i ) {
                env_size += i->first.size() + i->second.size() + 2; // = and \0
            }
            result.reserve(env_size);
        }
        
        for( environment_t::const_iterator i = env.begin(); i != env.end(); ++i ) {
            
            result.insert(result.end(), i->first.begin(), i->first.end());
            
            result.insert(result.end(), '=');
            
            result.insert(result.end(), i->second.begin(), i->second.end());
            
            result.insert(result.end(), '\0');
        }
        
        result.insert(result.end(), '\0');
    }
};

} // end namespace win32

std::auto_ptr<subprocess> subprocess::create()
{
    return std::auto_ptr<subprocess>(new win32::win32_subprocess());
}

} // end namespace tinfra::win32
