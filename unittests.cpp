
///
/// driver for all UnitTest++ driven tests
///

#include <unittest++/UnitTest++.h>
#include "tinfra/cmd.h"

int test_main(int argc, char** argv)
{
    return UnitTest::RunAllTests();
}

int main(int argc, char** argv)
{
    return tinfra::cmd::main(argc,argv, test_main);
}

