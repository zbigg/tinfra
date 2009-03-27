//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

///
/// driver for all tinfra unittests driven tests
///

#include <string>
#include <vector>

#include "tinfra/test.h"
#include "tinfra/cmd.h"

#ifdef SRCDIR
static std::string test_resources_dir = SRCDIR "/tests/resources";
#else
static std::string test_resources_dir =  "tests/resources";
#endif

int main(int argc, char** argv)
{
    if( argc > 2 && std::string(argv[1]) == "--test-resources-dir" )
        tinfra::test::set_test_resources_dir(argv[2]);
    else
        tinfra::test::set_test_resources_dir(test_resources_dir);
    return tinfra::cmd::main_wrapper(argc,argv, tinfra::test::test_main);
}

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:
