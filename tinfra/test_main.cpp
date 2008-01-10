///
/// driver for all UnitTest++ driven tests
///

#include <unittest++/UnitTest++.h>
#include <string>

#include "tinfra/test.h"
#include "tinfra/cmd.h"

#include <unittest++/TestReporter.h>
#include <unittest++/TestDetails.h>
#include <cstdio>
#include <cstddef>

#ifdef _WIN32
#include <windows.h>
#endif

static void out(const char* message, ...)
{
    va_list ap;
    va_start(ap, message);
#ifdef _WIN32
    char buf[2048];
    std::vsprintf(buf,message, ap);
    std::printf("%s",buf);
    OutputDebugString(buf);
#else
    std::vfprintf(stdout, message, ap);
#endif
    va_end(ap);
}

class TinfraTestReporter: public UnitTest::TestReporter {
public:    
    void ReportFailure(UnitTest::TestDetails const& details, char const* failure)
    {

#ifdef __APPLE__
        char const* const errorFormat = "%s:%d: error: FAILURE in %s: %s\n";
#else
        char const* const errorFormat = "%s(%d): error: FAILURE in %s: %s\n";
#endif
        out(errorFormat, details.filename, details.lineNumber, details.testName, failure);
    }

    void ReportTestStart(UnitTest::TestDetails const& test)
    {
        if( strcmp("DefaultSuite",test.suiteName) != 0 ) {
            out("TEST %s::%s\n", test.suiteName, test.testName);
        } else {
            out("TEST %s\n", test.testName);
        }
    }

    void ReportTestFinish(UnitTest::TestDetails const& /*test*/, float)
    {
    }

    void ReportSummary(int totalTestCount, int failedTestCount,
                       int failureCount, float secondsElapsed)
    {
        if (failureCount > 0)
            out("FAILURE: %d out of %d tests failed (%d failures).\n", failedTestCount, totalTestCount, failureCount);
        else
            out("Success: %d tests passed.\n", totalTestCount);
        out("Test time: %.2f seconds.\n", secondsElapsed);
    }
};



#ifdef SRCDIR
static std::string test_resources_dir = SRCDIR "/tinfra/test_resources";
#else
static std::string test_resources_dir =  "tinfra/test_resources";
#endif

int test_main(int argc, char** argv)
{
    TinfraTestReporter reporter;
    return UnitTest::RunAllTests(reporter, UnitTest::Test::GetTestList(), 0);
}

int main(int argc, char** argv)
{
    if( argc > 2 && std::string(argv[1]) == "--test-resources-dir" )
        tinfra::test::TempTestLocation::setTestResourcesDir(argv[2]);
    else
        tinfra::test::TempTestLocation::setTestResourcesDir(test_resources_dir);
    return tinfra::cmd::main(argc,argv, test_main);
}

