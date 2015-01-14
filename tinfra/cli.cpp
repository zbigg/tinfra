//
// Copyright (c) 2010-2011, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "cli.h" // we implement this

#include "path.h"  // we use these from tinfras
#include "runtime.h"
#include "fmt.h"

namespace tinfra {

tinfra::option_switch opt_help('h', "help", "show available options");
    
int cli_main(int argc, char** argv, int (*program_main)(tstring const& program_name, std::vector<tinfra::tstring>& args))
{    
    std::vector<tinfra::tstring> args(argv+1, argv+argc);
    tinfra::option_registry::get().parse(args);
    
    if( opt_help.enabled() ) {
        using tinfra::path::basename;
        using tinfra::get_exepath;
        std::string usage_header = tsprintf(
            "Usage: %s [options] [ test_case ... ]\n"
            "Available options:\n", basename(get_exepath()));
        
        tinfra::out.write(tstring(usage_header));
        tinfra::option_registry::get().print_help(tinfra::out);
        return 0;
    }
    
    return program_main(argv[0], args);
}

} // end namespace tinfra
