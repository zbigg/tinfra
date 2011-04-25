#include "tinfra/regexp.h"
#include "tinfra/cmd.h"
#include "tinfra/trace.h"
#include "tinfra/fmt.h" 
#include <cassert>
#include <string>
#include <iostream>

TINFRA_PUBLIC_TRACER(guidtool);

int guidtool_main(int argc, char** argv)
{
    TINFRA_USE_TRACER(guidtool);
    assert(argc == 2);
    std::string input = argv[1];
    
    
    // we parse this input
    // {E70C92A9-4BFD-11d1-8A95-00C04FB951F3}
    const tinfra::regexp guid_re("(\\w{8})-(\\w{4})-(\\w{4})-(\\w{4})-(\\w{12})");
    std::string gpre,g2,g3,g4,grest;
    
    bool matches = tinfra::scanner(guid_re, input.c_str()) % gpre % g2 % g3 % g4 % grest;
    if( !matches ) {
        TINFRA_LOG_ERROR("bad guid");
        return 0;
    }
    
    TINFRA_TRACE_VAR(gpre);
    TINFRA_TRACE_VAR(g2);
    TINFRA_TRACE_VAR(g3);
    TINFRA_TRACE_VAR(g4);
    TINFRA_TRACE_VAR(grest);
    
    // we output something like this:
    //DEFINE_GUID(CLSID_StoreNamespace,                 0xe70c92a9, 0x4bfd, 0x11d1, 0x8a, 0x95, 0x0, 0xc0, 0x4f, 0xb9, 0x51, 0xf3); 
    std::cout << tinfra::fmt("DEFINE_GUID(_class_name_, 0x%s, 0x%s, 0x%s, 0x%s, 0x%s, 0x%s, 0x%s, 0x%s, 0x%s, 0x%s, 0x%s);\n")
        % gpre % g2 % g3
        % g4.substr(0,2)
        % g4.substr(2,2)
        % grest.substr(0,2)
        % grest.substr(2,2)
        % grest.substr(4,2)
        % grest.substr(6,2)
        % grest.substr(8,2)
        % grest.substr(10,2);
        
    
    return 0;
}


TINFRA_MAIN(guidtool_main);
