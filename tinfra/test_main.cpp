///
/// driver for all UnitTest++ driven tests
///

#include <unittest++/UnitTest++.h>
#include <string>

#include "tinfra/test.h"
#include "tinfra/cmd.h"

#ifdef SRCDIR
static std::string test_resources_dir = SRCDIR "/tinfra/test_resources";
#else
static std::string test_resources_dir =  "tinfra/test_resources";
#endif

int test_main(int argc, char** argv)
{
    return UnitTest::RunAllTests();
}

int main(int argc, char** argv)
{
    if( argc > 2 && std::string(argv[1]) == "--test-resources-dir" )
        tinfra::test::TempTestLocation::setTestResourcesDir(argv[2]);
    else
        tinfra::test::TempTestLocation::setTestResourcesDir(test_resources_dir);
    return tinfra::cmd::main(argc,argv, test_main);
}

