//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

///
/// fatal exception handling for posix-like systems
///
/// based on codebase from
///    http://win32.mvps.org/misc/stackwalk.html

#include "../platform.h"
#ifdef TINFRA_W32

#include "tinfra/exeinfo.h"
#include "tinfra/thread.h"
#include "tinfra/runtime.h"
#include "tinfra/trace.h"
#include "tinfra/fmt.h"
#include "tinfra/buffer.h"
#include "tinfra/logger.h"

#include <windows.h>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#include <imagehlp.h>

#define gle (GetLastError())
#define lenof(a) (sizeof(a) / sizeof((a)[0]))
#define MAXNAMELEN 1024 // max name length for found symbols
#define IMGSYMLEN ( sizeof(IMAGEHLP_SYMBOL) )
#define TTBUFLEN 65536 // for a temp buffer

#ifdef _MSC_VER
#pragma warning( disable: 4311) // 'type cast' : pointer truncation from 'BYTE *' to 'DWORD'
                                // because it's meaningfull only on win64 platform
                                // and we don't support it yet
#endif

#ifdef _WIN64
#error "fatal_exception not implemented for win64" 
#endif

// SymCleanup()
typedef BOOL (__stdcall *tSymCleanup)( IN HANDLE hProcess );
tSymCleanup pSymCleanup = NULL;

// SymFunctionTableAccess()
typedef PVOID (__stdcall *tSymFunctionTableAccess)( HANDLE hProcess, DWORD AddrBase );
tSymFunctionTableAccess pSymFunctionTableAccess = NULL;

// SymGetLineFromAddr()
typedef BOOL (__stdcall *tSymGetLineFromAddr)( IN HANDLE hProcess, IN DWORD dwAddr,
	OUT PDWORD pdwDisplacement, OUT PIMAGEHLP_LINE Line );
tSymGetLineFromAddr pSymGetLineFromAddr = NULL;

// SymGetModuleBase()
typedef DWORD (__stdcall *tSymGetModuleBase)( IN HANDLE hProcess, IN DWORD dwAddr );
tSymGetModuleBase pSymGetModuleBase = NULL;

// SymGetModuleInfo()
typedef BOOL (__stdcall *tSymGetModuleInfo)( IN HANDLE hProcess, IN DWORD dwAddr, OUT PIMAGEHLP_MODULE ModuleInfo );
tSymGetModuleInfo pSymGetModuleInfo = NULL;

// SymGetOptions()
typedef DWORD (__stdcall *tSymGetOptions)( VOID );
tSymGetOptions pSymGetOptions = NULL;

// SymGetSymFromAddr()
typedef BOOL (__stdcall *tSymGetSymFromAddr)( IN HANDLE hProcess, IN DWORD dwAddr,
	OUT PDWORD pdwDisplacement, OUT PIMAGEHLP_SYMBOL Symbol );
tSymGetSymFromAddr pSymGetSymFromAddr = NULL;

// SymInitialize()
typedef BOOL (__stdcall *tSymInitialize)( IN HANDLE hProcess, IN PSTR UserSearchPath, IN BOOL fInvadeProcess );
tSymInitialize pSymInitialize = NULL;

// SymLoadModule()
typedef DWORD (__stdcall *tSymLoadModule)( IN HANDLE hProcess, IN HANDLE hFile,
	IN PSTR ImageName, IN PSTR ModuleName, IN DWORD BaseOfDll, IN DWORD SizeOfDll );
tSymLoadModule pSymLoadModule = NULL;

// SymSetOptions()
typedef DWORD (__stdcall *tSymSetOptions)( IN DWORD SymOptions );
tSymSetOptions pSymSetOptions = NULL;

// StackWalk()
typedef BOOL (__stdcall *tStackWalk)( DWORD MachineType, HANDLE hProcess,
	HANDLE hThread, LPSTACKFRAME StackFrame, PVOID ContextRecord,
	PREAD_PROCESS_MEMORY_ROUTINE ReadMemoryRoutine,
	PFUNCTION_TABLE_ACCESS_ROUTINE FunctionTableAccessRoutine,
	PGET_MODULE_BASE_ROUTINE GetModuleBaseRoutine,
	PTRANSLATE_ADDRESS_ROUTINE TranslateAddress );
tStackWalk pStackWalk = NULL;

// UnDecorateSymbolName()
typedef DWORD (__stdcall WINAPI *tUnDecorateSymbolName)( PCSTR DecoratedName, PSTR UnDecoratedName,
	DWORD UndecoratedLength, DWORD Flags );
tUnDecorateSymbolName pUnDecorateSymbolName = NULL;

struct ModuleEntry
{
	std::string imageName;
	std::string moduleName;
	DWORD baseAddress;
	DWORD size;
};
typedef std::vector< ModuleEntry > ModuleList;
typedef ModuleList::iterator ModuleListIter;

static DWORD unhandled_exception_filter( EXCEPTION_POINTERS *ep );
static void enumAndLoadModuleSymbols( HANDLE hProcess, DWORD pid );
static bool fillModuleList( ModuleList& modules, DWORD pid, HANDLE hProcess );
static bool fillModuleListTH32( ModuleList& modules, DWORD pid );
static bool fillModuleListPSAPI( ModuleList& modules, DWORD pid, HANDLE hProcess );

static bool initialize_imaghelp();
static bool initialize_sym(HANDLE hProcess);

static bool get_current_thread_stacktrace(tinfra::stacktrace_t& dest);
static bool get_thread_stacktace(HANDLE hThread, CONTEXT& c, tinfra::stacktrace_t& dest );

BOOL WINAPI ConsoleHandler(DWORD)
{
    /* TODO: should we differentiate anyhow ?
    switch(event)
    {
    case CTRL_C_EVENT:
        break;
    case CTRL_BREAK_EVENT:
        break;
    case CTRL_CLOSE_EVENT:
        break;
    case CTRL_LOGOFF_EVENT:
        break;
    case CTRL_SHUTDOWN_EVENT:
        break;
    }
    */
    tinfra::interrupt();
    return TRUE;
}

static DWORD unhandled_exception_filter( EXCEPTION_POINTERS *ep )
{
    // TODO: this should be removed and stacktrace should be collected
    //       not printed
    //printf( "%s[%i]: Fatal exception occurred.\n", tinfra::get_exepath().c_str(), tinfra::Thread::current().to_number());
    //printf( "Call stack:\n");
    
    
    tinfra::stacktrace_t trace;
    // TODO: trace should be filled and passed to fatal_exit
    get_thread_stacktace( GetCurrentThread(), *(ep->ContextRecord), trace );
    
    // TODO: abort() or return here ?
    tinfra::fatal_exit("fatal exception",trace);
    return EXCEPTION_EXECUTE_HANDLER;
}

static HANDLE tinfra_hOurProcess;

namespace tinfra {



void initialize_platform_runtime()
{    	
    tinfra_hOurProcess = GetCurrentProcess();
    if( !initialize_imaghelp() ) {
	TINFRA_LOG_ERROR("unable to initialize imaghelp");
    }

    if( !initialize_sym(tinfra_hOurProcess) ) {
	TINFRA_LOG_ERROR("unable to initialize symbol reader");
    }

    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)unhandled_exception_filter);
    
    if( !SetConsoleCtrlHandler(ConsoleHandler, TRUE) ) {
	TINFRA_LOG_ERROR("unable to initialize contole interrupt handler");
    }
}

void uninstall_abort_handler()
{
    // not sure, how to do this on win32
}

bool is_stacktrace_supported()
{
    return true;
}


bool get_debug_info(void* address, debug_info& result)
{
    if ( pSymGetLineFromAddr == NULL ) {
	return false;
    }

    HANDLE hProcess = tinfra_hOurProcess; // hProcess normally comes from outside
    int frameNum; // counts walked frames
    DWORD offsetFromSymbol; // tells us how far from the symbol we were
    
    IMAGEHLP_SYMBOL *pSym = (IMAGEHLP_SYMBOL *) malloc( IMGSYMLEN + MAXNAMELEN );
    char undFullName[MAXNAMELEN]; // undecorated name with all shenanigans
    IMAGEHLP_LINE Line;
    
    result.source_line = -1;
    if ( ! pSymGetSymFromAddr( hProcess, (DWORD)address, &offsetFromSymbol, pSym ) ) {
        if ( gle != 487 )
            TINFRA_LOG_ERROR(fmt("get_debug_info: SymGetSymFromAddr failed, gle=%i") % gle);
	    free(pSym);
	    return false;
    } else {

        pUnDecorateSymbolName( pSym->Name, undFullName, MAXNAMELEN, UNDNAME_COMPLETE );
        result.function = undFullName;
    }	    
    
    if ( ! pSymGetLineFromAddr( hProcess, (DWORD)address, &offsetFromSymbol, &Line ) ) {
	if ( gle != 487 ) {
	    TINFRA_LOG_ERROR(fmt("get_debug_info: pSymGetLineFromAddr failed, gle=%i") % gle);
	}
	free(pSym);
	return false;
    }

    result.source_line = Line.LineNumber;
    result.source_file = Line.FileName;
    free(pSym);
    return true;
}

bool get_stacktrace(stacktrace_t& th) 
{
    return get_current_thread_stacktrace(th);
}

} // end namespace tinfra

//
// implementation details
// 

// this method is called 
// from separate, temporary thread to inspect
// thread in which exception occured.
//

struct  thread_callstack_dumper_args {
    HANDLE                hThread;
    bool                  finished;
    tinfra::stacktrace_t* trace_result;
};

static DWORD __stdcall thread_callstack_dumper( void * arg )
{
    thread_callstack_dumper_args* args = (thread_callstack_dumper_args*)arg;    
    
    ::CONTEXT context;
    memset( &context, '\0', sizeof(::CONTEXT) );
    context.ContextFlags = CONTEXT_FULL;

    ::SuspendThread(args->hThread);
    if ( ! ::GetThreadContext( args->hThread, &context ) ) {
        printf("unable to read thread context, call stack dump failed");
    } else {
        if( !get_thread_stacktace( args->hThread, context, *(args->trace_result) ) ) {
            // TODO: handle error
        }
    }
    ::ResumeThread(args->hThread);
    args->finished = true;
    return 0;
}

static bool get_current_thread_stacktrace(tinfra::stacktrace_t& dest)
{
    thread_callstack_dumper_args args;
    
    if( ::DuplicateHandle(::GetCurrentProcess(),
                    ::GetCurrentThread(),
                    ::GetCurrentProcess(),
                    &args.hThread,
                    0, FALSE, DUPLICATE_SAME_ACCESS) == 0 )
    {
        TINFRA_LOG_ERROR("get_current_thread_stacktrace: unable to obtain thread handle");
        return false;
    }
    args.finished = 0;
    args.trace_result = &dest;
    
    // TODO - use beginthreadex or better, tinfra::thread
    HANDLE hDumpThread = ::CreateThread( NULL, 5*524288, thread_callstack_dumper,  &args, 0, 0 );
    if( hDumpThread == NULL ) {
        ::CloseHandle(args.hThread);
        printf("get_current_thread_stacktrace: unable to create thread");
        return false;
    }
    
    {   // let dumper thread suspend us and wait half-actively for 
        // "job done" signal
        int old_thread_priotity = GetThreadPriority(GetCurrentThread());
        ::SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_IDLE );
        while( !args.finished) {
            ::Sleep( 20 );
        }
        ::SetThreadPriority( GetCurrentThread(), old_thread_priotity );
    }
    {
        // wait for child thread
        ::WaitForSingleObject( hDumpThread, INFINITE );
        ::CloseHandle(hDumpThread);
        ::CloseHandle(args.hThread);
    }
    return true;
}

static bool get_thread_stacktace(HANDLE hThread, CONTEXT& c, tinfra::stacktrace_t& dest )
{
    // normally, call ImageNtHeader() and use machine info from PE header
    DWORD imageType = IMAGE_FILE_MACHINE_I386;
    HANDLE hProcess = tinfra_hOurProcess; // hProcess normally comes from outside
    int frameNum; // counts walked frames
    DWORD offsetFromSymbol; // tells us how far from the symbol we were
    
    IMAGEHLP_SYMBOL *pSym = (IMAGEHLP_SYMBOL *) malloc( IMGSYMLEN + MAXNAMELEN );
    char undFullName[MAXNAMELEN]; // undecorated name with all shenanigans
    
    IMAGEHLP_MODULE Module;
    IMAGEHLP_LINE Line;

    STACKFRAME stack_frame; // in/out stackframe
    memset( &stack_frame, '\0', sizeof stack_frame );

    DWORD address = 0;

    // NOTE: normally, the exe directory and the current directory should be taken
    // from the target process. The current dir would be gotten through injection
    // of a remote thread; the exe fir through either ToolHelp32 or PSAPI.

    

    // Enumerate modules and tell imagehlp.dll about them.
    // On NT, this is not necessary, but it won't hurt.
    //enumAndLoadModuleSymbols( hProcess, GetCurrentProcessId() );

    // init STACKFRAME for first call
    // Notes: AddrModeFlat is just an assumption. I hate VDM debugging.
    // Notes: will have to be #ifdef-ed for Alphas; MIPSes are dead anyway,
    // and good riddance.
    stack_frame.AddrPC.Offset = c.Eip;
    stack_frame.AddrPC.Mode = AddrModeFlat;
    stack_frame.AddrFrame.Offset = c.Ebp;
    stack_frame.AddrFrame.Mode = AddrModeFlat;

    memset( pSym, '\0', IMGSYMLEN + MAXNAMELEN );
    pSym->SizeOfStruct = IMGSYMLEN;
    pSym->MaxNameLength = MAXNAMELEN;

    memset( &Line, '\0', sizeof Line );
    Line.SizeOfStruct = sizeof Line;

    memset( &Module, '\0', sizeof Module );
    Module.SizeOfStruct = sizeof Module;

    offsetFromSymbol = 0;
    
  

    for ( frameNum = 0; ; ++ frameNum )
    {
	
	char* module_name = 0;
	DWORD  module_base = 0;
	
	char* symbol_name = 0;
	int   symbol_offset = -1;
	char* file_name = 0;
	int   line_number = -1;
	
	// get next stack frame (StackWalk(), SymFunctionTableAccess(), SymGetModuleBase())
	// if this returns ERROR_INVALID_ADDRESS (487) or ERROR_NOACCESS (998), you can
	// assume that either you are done, or that the stack is so hosed that the next
	// deeper frame could not be found.
	if ( ! pStackWalk( imageType, hProcess, hThread, &stack_frame, &c, NULL,
		pSymFunctionTableAccess, pSymGetModuleBase, NULL ) )
	    break;
	
	if( address == stack_frame.AddrPC.Offset )
	    break;
	address = stack_frame.AddrPC.Offset;

	if( stack_frame.AddrPC.Offset == 0 || stack_frame.AddrReturn.Offset == 0) break;
		
        dest.push_back((void*)address);

    } // for ( frameNum )

cleanup:
    ResumeThread( hThread );
    // de-init symbol handler etc. (SymCleanup())
    pSymCleanup( hProcess );
    free( pSym );
    
    return true;
}

static bool initialize_imaghelp()
{
    HINSTANCE hImagehlpDll = LoadLibrary( "dbghelp.dll" );
    if ( hImagehlpDll == NULL ) {
	TINFRA_LOG_ERROR( "unable to load dbghelp.dll" );
	return false;
    }
    
    // we load imagehlp.dll dynamically because the NT4-version does not
    // offer all the functions that are in the NT5 lib
    
    pSymCleanup = (tSymCleanup) GetProcAddress( hImagehlpDll, "SymCleanup" );
    pSymFunctionTableAccess = (tSymFunctionTableAccess) GetProcAddress( hImagehlpDll, "SymFunctionTableAccess" );
    pSymGetLineFromAddr = (tSymGetLineFromAddr) GetProcAddress( hImagehlpDll, "SymGetLineFromAddr" );
    pSymGetModuleBase = (tSymGetModuleBase) GetProcAddress( hImagehlpDll, "SymGetModuleBase" );
    pSymGetModuleInfo = (tSymGetModuleInfo) GetProcAddress( hImagehlpDll, "SymGetModuleInfo" );
    pSymGetOptions = (tSymGetOptions) GetProcAddress( hImagehlpDll, "SymGetOptions" );
    pSymGetSymFromAddr = (tSymGetSymFromAddr) GetProcAddress( hImagehlpDll, "SymGetSymFromAddr" );
    pSymInitialize = (tSymInitialize) GetProcAddress( hImagehlpDll, "SymInitialize" );
    pSymSetOptions = (tSymSetOptions) GetProcAddress( hImagehlpDll, "SymSetOptions" );
    pStackWalk = (tStackWalk) GetProcAddress( hImagehlpDll, "StackWalk" );
    pUnDecorateSymbolName = (tUnDecorateSymbolName) GetProcAddress( hImagehlpDll, "UnDecorateSymbolName" );
    pSymLoadModule = (tSymLoadModule) GetProcAddress( hImagehlpDll, "SymLoadModule" );
    
    if ( pSymCleanup == NULL || pSymFunctionTableAccess == NULL || pSymGetModuleBase == NULL || pSymGetModuleInfo == NULL ||
	    pSymGetOptions == NULL || pSymGetSymFromAddr == NULL || pSymInitialize == NULL || pSymSetOptions == NULL ||
	    pStackWalk == NULL || pUnDecorateSymbolName == NULL || pSymLoadModule == NULL )
    {
	TINFRA_LOG_ERROR( "GetProcAddress(): some required function not found in dbghelp.dll." );
	FreeLibrary( hImagehlpDll );
	return false;
    }
    return true;
}

static bool initialize_sym(HANDLE hProcess)
{
     // this is a _sample_. you can do the error checking yourself.
    char* tt = new char[TTBUFLEN];
    char* p = 0;
    // build symbol search path from:
    std::string symSearchPath = "";
    // current directory
    if ( GetCurrentDirectory( TTBUFLEN, tt ) )
	    symSearchPath += tt + std::string( ";" );
    // dir with executable
    if ( GetModuleFileName( 0, tt, TTBUFLEN ) )
    {
	    for ( p = tt + strlen( tt ) - 1; p >= tt; -- p )
	    {
		    // locate the rightmost path separator
		    if ( *p == '\\' || *p == '/' || *p == ':' )
			    break;
	    }
	    // if we found one, p is pointing at it; if not, tt only contains
	    // an exe name (no path), and p points before its first byte
	    if ( p != tt ) // path sep found?
	    {
		    if ( *p == ':' ) // we leave colons in place
			    ++ p;
		    *p = '\0'; // eliminate the exe name and last path sep
		    symSearchPath += tt + std::string( ";" );
	    }
    }
    // environment variable _NT_SYMBOL_PATH
    if ( GetEnvironmentVariable( "_NT_SYMBOL_PATH", tt, TTBUFLEN ) )
	    symSearchPath += tt + std::string( ";" );
    // environment variable _NT_ALTERNATE_SYMBOL_PATH
    if ( GetEnvironmentVariable( "_NT_ALTERNATE_SYMBOL_PATH", tt, TTBUFLEN ) )
	    symSearchPath += tt + std::string( ";" );
    // environment variable SYSTEMROOT
    if ( GetEnvironmentVariable( "SYSTEMROOT", tt, TTBUFLEN ) )
	    symSearchPath += tt + std::string( ";" );

    if ( symSearchPath.size() > 0 ) // if we added anything, we have a trailing semicolon
	    symSearchPath = symSearchPath.substr( 0, symSearchPath.size() - 1 );

    //printf( "symbols path: %s\n", symSearchPath.c_str() );

    // why oh why does SymInitialize() want a writeable string?
    char* tmpSymSearchPath = new char[symSearchPath.size()+1];
    strcpy( tmpSymSearchPath, symSearchPath.c_str());
    bool result = true;
    
    // init symbol handler stuff (SymInitialize())
    if ( ! pSymInitialize( hProcess, tmpSymSearchPath, false ) )
    {
	printf( "SymInitialize(): gle = %lu\n", gle );
	result = false;
	goto cleanup;
    }
    
    { // SymSetOptions call
	DWORD symOptions; // symbol handler settings
	// SymGetOptions()
	symOptions = pSymGetOptions();
	symOptions |= SYMOPT_LOAD_LINES;
	symOptions &= ~SYMOPT_UNDNAME;
	pSymSetOptions( symOptions ); // SymSetOptions()
    }
cleanup:
    delete[] tmpSymSearchPath;
    return result;
}

static void enumAndLoadModuleSymbols( HANDLE hProcess, DWORD pid )
{
    ModuleList modules;
    ModuleListIter it;
    char *img, *mod;

    // fill in module list
    fillModuleList( modules, pid, hProcess );

    for ( it = modules.begin(); it != modules.end(); ++ it )
    {
	// unfortunately, SymLoadModule() wants writeable strings
	img = new char[(*it).imageName.size() + 1];
	strcpy( img, (*it).imageName.c_str() );
	mod = new char[(*it).moduleName.size() + 1];
	strcpy( mod, (*it).moduleName.c_str() );

	if ( pSymLoadModule( hProcess, 0, img, mod, (*it).baseAddress, (*it).size ) == 0 )
		printf( "Error %lu loading symbols for \"%s\"\n",
			gle, (*it).moduleName.c_str() );
	//else
	//	printf( "Symbols loaded: \"%s\"\n", (*it).moduleName.c_str() );

	delete [] img;
	delete [] mod;
    }
}



static bool fillModuleList( ModuleList& modules, DWORD pid, HANDLE hProcess )
{
    // try toolhelp32 first
    if ( fillModuleListTH32( modules, pid ) )
	    return true;
    // nope? try psapi, then
    return fillModuleListPSAPI( modules, pid, hProcess );
}



// miscellaneous toolhelp32 declarations; we cannot #include the header
// because not all systems may have it
#define MAX_MODULE_NAME32 255
#define TH32CS_SNAPMODULE   0x00000008
#pragma pack( push, 8 )
typedef struct tagMODULEENTRY32
{
    DWORD   dwSize;
    DWORD   th32ModuleID;       // This module
    DWORD   th32ProcessID;      // owning process
    DWORD   GlblcntUsage;       // Global usage count on the module
    DWORD   ProccntUsage;       // Module usage count in th32ProcessID's context
    BYTE  * modBaseAddr;        // Base address of module in th32ProcessID's context
    DWORD   modBaseSize;        // Size in bytes of module starting at modBaseAddr
    HMODULE hModule;            // The hModule of this module in th32ProcessID's context
    char    szModule[MAX_MODULE_NAME32 + 1];
    char    szExePath[MAX_PATH];
} MODULEENTRY32;
typedef MODULEENTRY32 *  PMODULEENTRY32;
typedef MODULEENTRY32 *  LPMODULEENTRY32;
#pragma pack( pop )



static bool fillModuleListTH32( ModuleList& modules, DWORD pid )
{
    // CreateToolhelp32Snapshot()
    typedef HANDLE (__stdcall *tCT32S)( DWORD dwFlags, DWORD th32ProcessID );
    // Module32First()
    typedef BOOL (__stdcall *tM32F)( HANDLE hSnapshot, LPMODULEENTRY32 lpme );
    // Module32Next()
    typedef BOOL (__stdcall *tM32N)( HANDLE hSnapshot, LPMODULEENTRY32 lpme );

    // I think the DLL is called tlhelp32.dll on Win9X, so we try both
    const char *dllname[] = { "kernel32.dll", "tlhelp32.dll" };
    HINSTANCE hToolhelp = 0;
    tCT32S pCT32S = 0;
    tM32F pM32F = 0;
    tM32N pM32N = 0;

    HANDLE hSnap;
    MODULEENTRY32 me = { sizeof me };
    bool keepGoing;
    ModuleEntry e;    

    for ( unsigned i = 0; i < lenof( dllname ); ++ i )
    {
	hToolhelp = LoadLibrary( dllname[i] );
	if ( hToolhelp == 0 )
	    continue;
	pCT32S = (tCT32S) GetProcAddress( hToolhelp, "CreateToolhelp32Snapshot" );
	pM32F = (tM32F) GetProcAddress( hToolhelp, "Module32First" );
	pM32N = (tM32N) GetProcAddress( hToolhelp, "Module32Next" );
	if ( pCT32S != 0 && pM32F != 0 && pM32N != 0 )
		break; // found the functions!
	FreeLibrary( hToolhelp );
	hToolhelp = 0;
    }

    if ( hToolhelp == 0 ) // nothing found?
	return false;

    hSnap = pCT32S( TH32CS_SNAPMODULE, pid );
    if ( hSnap == (HANDLE) -1 )
	return false;

    keepGoing = !!pM32F( hSnap, &me );
    while ( keepGoing )
    {
	// here, we have a filled-in MODULEENTRY32
	//printf( "%08lXh %6lu %-15.15s %s\n", (unsigned long)me.modBaseAddr, me.modBaseSize, me.szModule, me.szExePath );
	e.imageName = me.szExePath;
	e.moduleName = me.szModule;
	e.baseAddress = (DWORD) me.modBaseAddr;
	e.size = me.modBaseSize;
	modules.push_back( e );
	keepGoing = !!pM32N( hSnap, &me );
    }

    CloseHandle( hSnap );

    FreeLibrary( hToolhelp );

    return modules.size() != 0;
}



// miscellaneous psapi declarations; we cannot #include the header
// because not all systems may have it
typedef struct _MODULEINFO {
    LPVOID lpBaseOfDll;
    DWORD SizeOfImage;
    LPVOID EntryPoint;
} MODULEINFO, *LPMODULEINFO;



static bool fillModuleListPSAPI( ModuleList& modules, DWORD, HANDLE hProcess )
{
    // EnumProcessModules()
    typedef BOOL (__stdcall *tEPM)( HANDLE hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded );
    // GetModuleFileNameEx()
    typedef DWORD (__stdcall *tGMFNE)( HANDLE hProcess, HMODULE hModule, LPSTR lpFilename, DWORD nSize );
    // GetModuleBaseName() -- redundant, as GMFNE() has the same prototype, but who cares?
    typedef DWORD (__stdcall *tGMBN)( HANDLE hProcess, HMODULE hModule, LPSTR lpFilename, DWORD nSize );
    // GetModuleInformation()
    typedef BOOL (__stdcall *tGMI)( HANDLE hProcess, HMODULE hModule, LPMODULEINFO pmi, DWORD nSize );

    HINSTANCE hPsapi;
    tEPM pEPM;
    tGMFNE pGMFNE;
    tGMBN pGMBN;
    tGMI pGMI;
    
    ModuleEntry e;
    DWORD cbNeeded;
    MODULEINFO mi;
    HMODULE *hMods = 0;
    char *tt = 0;

    unsigned i;
    
    hPsapi = LoadLibrary( "psapi.dll" );
    if ( hPsapi == 0 )
	return false;

    modules.clear();

    pEPM = (tEPM) GetProcAddress( hPsapi, "EnumProcessModules" );
    pGMFNE = (tGMFNE) GetProcAddress( hPsapi, "GetModuleFileNameExA" );
    pGMBN = (tGMFNE) GetProcAddress( hPsapi, "GetModuleBaseNameA" );
    pGMI = (tGMI) GetProcAddress( hPsapi, "GetModuleInformation" );
    if ( pEPM == 0 || pGMFNE == 0 || pGMBN == 0 || pGMI == 0 )
    {
	// yuck. Some API is missing.
	FreeLibrary( hPsapi );
	return false;
    }

    hMods = new HMODULE[TTBUFLEN / sizeof(HMODULE)];
    tt = new char[TTBUFLEN];
    // not that this is a sample. Which means I can get away with
    // not checking for errors, but you cannot. :)

    if ( ! pEPM( hProcess, hMods, TTBUFLEN, &cbNeeded ) )
    {
	printf( "EPM failed, gle = %lu\n", gle );
	goto cleanup;
    }

    if ( cbNeeded > TTBUFLEN )
    {
	printf( "More than %i module handles. Huh?\n", lenof( hMods ) );
	goto cleanup;
    }

    for ( i = 0; i < cbNeeded / sizeof hMods[0]; ++ i )
    {
	// for each module, get:
	// base address, size
	pGMI( hProcess, hMods[i], &mi, sizeof mi );
	e.baseAddress = (DWORD) mi.lpBaseOfDll;
	e.size = mi.SizeOfImage;
	// image file name
	tt[0] = '\0';
	pGMFNE( hProcess, hMods[i], tt, TTBUFLEN );
	e.imageName = tt;
	// module name
	tt[0] = '\0';
	pGMBN( hProcess, hMods[i], tt, TTBUFLEN );
	e.moduleName = tt;
	//printf( "%08lXh %6lu %-15.15s %s\n", e.baseAddress,
	//	e.size, e.moduleName.c_str(), e.imageName.c_str() );

	modules.push_back( e );
    }

cleanup:
    if ( hPsapi )
	FreeLibrary( hPsapi );
    delete [] tt;
    delete [] hMods;

    return modules.size() != 0;
}

#endif // TINFRA_W32

