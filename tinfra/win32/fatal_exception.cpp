
///
/// fatal exception handling for posix-like systems
///
/// based on codebase from
///    http://win32.mvps.org/misc/stackwalk.html

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

static void show_stack( HANDLE hThread, CONTEXT& c ); // dump a stack
static DWORD unhandled_exception_filter( EXCEPTION_POINTERS *ep );
static void enumAndLoadModuleSymbols( HANDLE hProcess, DWORD pid );
static bool fillModuleList( ModuleList& modules, DWORD pid, HANDLE hProcess );
static bool fillModuleListTH32( ModuleList& modules, DWORD pid );
static bool fillModuleListPSAPI( ModuleList& modules, DWORD pid, HANDLE hProcess );

bool initialize_imaghelp()
{
    HINSTANCE hImagehlpDll = NULL;
    hImagehlpDll = LoadLibrary( "dbghelp.dll" );
    if ( hImagehlpDll == NULL ) {
	printf( "LoadLibrary( \"dbghelp.dll\" ): gle = %lu\n", gle );
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
	puts( "GetProcAddress(): some required function not found." );
	FreeLibrary( hImagehlpDll );
	return false;
    }
    return true;
}

static void (*fatal_exception_handler) (void) = 0;

static DWORD unhandled_exception_filter( EXCEPTION_POINTERS *ep )
{
    printf( "Fatal exception occurred.\n");
    printf( "Call stack:\n");
    show_stack( GetCurrentThread(), *(ep->ContextRecord) );    
    if( fatal_exception_handler ) {
	fatal_exception_handler( /* ??? */);
    }
    printf( "Terminating application.\n");    
    return EXCEPTION_EXECUTE_HANDLER;
}

namespace tinfra {
void initialize_fatal_exception_handler(void (*handler) (void))
{    
    initialize_imaghelp();
    fatal_exception_handler = handler;
    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)unhandled_exception_filter);
}
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
static void show_stack(HANDLE hThread, CONTEXT& c )
{
    // normally, call ImageNtHeader() and use machine info from PE header
    DWORD imageType = IMAGE_FILE_MACHINE_I386;
    HANDLE hProcess = GetCurrentProcess(); // hProcess normally comes from outside
    int frameNum; // counts walked frames
    DWORD offsetFromSymbol; // tells us how far from the symbol we were
    
    IMAGEHLP_SYMBOL *pSym = (IMAGEHLP_SYMBOL *) malloc( IMGSYMLEN + MAXNAMELEN );
    char undFullName[MAXNAMELEN]; // undecorated name with all shenanigans
    
    IMAGEHLP_MODULE Module;
    IMAGEHLP_LINE Line;

    STACKFRAME stack_frame; // in/out stackframe
    memset( &stack_frame, '\0', sizeof stack_frame );

    // NOTE: normally, the exe directory and the current directory should be taken
    // from the target process. The current dir would be gotten through injection
    // of a remote thread; the exe fir through either ToolHelp32 or PSAPI.

    if( !initialize_sym(hProcess) ) {
	goto cleanup;
    }

    

    // Enumerate modules and tell imagehlp.dll about them.
    // On NT, this is not necessary, but it won't hurt.
    enumAndLoadModuleSymbols( hProcess, GetCurrentProcessId() );

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
	DWORD address = 0;
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
	
	address = stack_frame.AddrPC.Offset;
	
	// display its contents
	//printf( "\n%3d %c%c %08lx %08lx %08lx %08lx ",
	//	frameNum, s.Far? 'F': '.', s.Virtual? 'V': '.',
	//	s.AddrPC.Offset, s.AddrReturn.Offset,
	//	s.AddrFrame.Offset, s.AddrStack.Offset );

	if( stack_frame.AddrPC.Offset == 0 || stack_frame.AddrReturn.Offset == 0) break;
	
	if ( ! pSymGetSymFromAddr( hProcess, address, &offsetFromSymbol, pSym ) ) {
	    if ( gle != 487 )
		    printf( "SymGetSymFromAddr(): gle = %lu\n", gle );
	} else {
	    // UnDecorateSymbolName()
	    //pUnDecorateSymbolName( pSym->Name, undName, MAXNAMELEN, UNDNAME_NAME_ONLY );
	    pUnDecorateSymbolName( pSym->Name, undFullName, MAXNAMELEN, UNDNAME_COMPLETE );
	    /*printf( "%s", undName );
	    if ( offsetFromSymbol != 0 )
		    printf( " %+ld bytes", (long) offsetFromSymbol );
	    putchar( '\n' );
	    printf( "    Sig:  %s\n", pSym->Name );
	    printf( "    Decl: %s\n", undFullName );*/
	    symbol_name = undFullName;
	}	    

	// show line number info, NT5.0-method (SymGetLineFromAddr())
	if ( pSymGetLineFromAddr != NULL ) { 
	    if ( ! pSymGetLineFromAddr( hProcess, address, &offsetFromSymbol, &Line ) )
	    {
		if ( gle != 487 )
		    printf( "SymGetLineFromAddr(): gle = %lu\n", gle );
	    }
	    else
	    {
		//printf( "    Line: %s(%lu) %+ld bytes\n",
		//	Line.FileName, Line.LineNumber, offsetFromSymbol );
		line_number = Line.LineNumber;
		file_name = Line.FileName;
		symbol_offset = offsetFromSymbol;
	    }
	}

	// show module info (SymGetModuleInfo())
	if ( ! pSymGetModuleInfo( hProcess, address, &Module ) )
	{
	    printf( "SymGetModuleInfo): gle = %lu\n", gle );
	}
	else { // got module info OK
	    module_name = Module.ModuleName;
	    module_base = Module.BaseOfImage;
	}
	
	printf("0x%08x", (unsigned int)address);
	if( module_name ) {
	    printf(" (%s)", module_name);	    
	}
	if( symbol_name ) {
	    printf(" %s", symbol_name);
	    if( symbol_offset != -1 ) {
		printf(" (+%i bytes)", symbol_offset);
	    }
	}
	if( file_name ) {
	    printf("(%s:%i)", file_name, line_number);
	}
	printf("\n");

    } // for ( frameNum )

cleanup:
    ResumeThread( hThread );
    // de-init symbol handler etc. (SymCleanup())
    pSymCleanup( hProcess );
    free( pSym );
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
    HINSTANCE hToolhelp;
    tCT32S pCT32S;
    tM32F pM32F;
    tM32N pM32N;

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



static bool fillModuleListPSAPI( ModuleList& modules, DWORD pid, HANDLE hProcess )
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
