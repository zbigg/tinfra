#include "tinfra/csv.h"


#include "tinfra/cmd.h"

#ifdef BUILD_UNITTEST
#include <unittest++/UnitTest++.h>
#include <unittest++/TestReporterStdout.h>

int importer_main(int, char**)
{
    return UnitTest::RunAllTests();
}
#else
int importer_main(int argc, char** argv)
{
    return 0;
}
#endif
TINFRA_MAIN(importer_main);
