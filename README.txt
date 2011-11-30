
= Content =

    There are two libraries:
    
    tinfra 
    
        Base tinfra library for use in normal code.
        No dependencies.
        
        API Headers: all but tinfra/test*.h
        LIB: libtinfra.a or tinfra.lib 
    
    tinfra-test
    
        Simple unit-test helper code inspired by UnitTest++ (http://unittest-cpp.sourceforge.net/)
        
        Headers: tinfra/test.h
        LIB: libtinfra-test.a or tinfra-test.lib 
        
    Tests of library are under unittests(.exe) target. They cover something like 50% of 
    current code and 20-30% of functionality.
    
        
= Usage, Linux/Unix or MSYS/Mingw = 

    To build:
        ./configure --prefix=???
        make
    
    To install
        make install
        
    (installs ${exec_prefix}/lib/libtinfra.a & libtinfra-test.a and ${prefix}/include/tinfra/...)
        
    To check:
        ./unittests
        
        (make check fails currently due to integration errors)
    
= Usage, MS VS 2008 and 2010 =

    Import tinfra_msvs2008.sln.

    Build unittests target.
    
    Run it.
    
    It should show something like:
    
        (...)
        TEST tinfra::text_line_reader_normal
        Success: 115 tests passed.
        Test time: 2.30 seconds.
        The thread 'Win32 Thread' (0x1cf8) has exited with code 0 (0x0).
        The program '[6992] unittests.exe: Native' has exited with code 0 (0x0).

    To use library it's enough to have distribution folder somewhere in INCLUDEPATH.
    
    Preferred way for #inclusion of tinfra headers is "#include tinfra/....h"
    
    On MSVS, libraries are built to:
    
        out/debug_vc/tinfra.lib         - base tinfra library for use in normal code
        out/debug_vc/tinfra-test.lib    - test helper code (depends on
	tinfra.lib)

