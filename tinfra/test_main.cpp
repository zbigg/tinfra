//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

///
/// driver for all UnitTest++ driven tests
///

#include <unittest++/UnitTest++.h>
#include <string>

#include "tinfra/test.h"
#include "tinfra/cmd.h"

#include <unittest++/TestReporter.h>
#include <unittest++/TestDetails.h>
#include <unittest++/TestResults.h>
#include <unittest++/TestReporter.h>

#include <stdio.h> // screw cxxx headers because they are incompatible
#include <stdarg.h>

#ifdef _WIN32
#include <windows.h>
#endif

static void out(const char* message, ...)
{
    va_list ap;
    va_start(ap, message);
#ifdef _WIN32
    char buf[2048];
    vsprintf(buf,message, ap);
    printf("%s",buf);
    OutputDebugString(buf);
#else
    vprintf(message, ap);
#endif
    va_end(ap);
}

class TinfraTestReporter: public UnitTest::TestReporter {
public:    
    void ReportFailure(UnitTest::TestDetails const& details, char const* failure)
    {

#ifdef _MSC_VER 
        char const* const errorFormat = "%s(%d): error: FAILURE in %s: %s\n";        
#else   // only MSC has weird message format, rest use name:line: message (AFAIK)
        char const* const errorFormat = "%s:%d: error: FAILURE in %s: %s\n";
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

UnitTest::Test const* search_test(UnitTest::TestList& tl, const char* name)
{
    UnitTest::Test const* test = tl.GetHead();
    while( test != 0 ) {
        if( std::strcmp(test->m_details.testName, name) == 0 )
            return test;
        test = test->next;
    }
    return 0;
}

int test_main(int argc, char** argv)
{
    TinfraTestReporter reporter;
    if( argc > 1 ) {
        UnitTest::TestResults result(&reporter);
        
        UnitTest::Timer overallTimer;
        overallTimer.Start();
    
        for( int i = 0; i < argc-1; ++i ) {
            const char* test_name = argv[i+1];
            UnitTest::Test const* current_test = search_test(UnitTest::Test::GetTestList(), test_name);
            if( !current_test ) {
                out("FAUILURE: test %s not found\n", test_name);
                continue;
            }
            
            UnitTest::Timer testTimer;
            testTimer.Start();
            
            result.OnTestStart(current_test->m_details);
            
            current_test->Run(result);
            
            int const testTimeInMs = testTimer.GetTimeInMs();
            
            result.OnTestFinish(current_test->m_details, testTimeInMs/1000.0f);
        } 
        
        float const secondsElapsed = overallTimer.GetTimeInMs() / 1000.0f;
        reporter.ReportSummary(result.GetTotalTestCount(), result.GetFailedTestCount(), result.GetFailureCount(), secondsElapsed);
    
        return result.GetFailureCount();
    } else {
        return UnitTest::RunAllTests(reporter, UnitTest::Test::GetTestList(), 0);
    }
}

int main(int argc, char** argv)
{
    if( argc > 2 && std::string(argv[1]) == "--test-resources-dir" )
        tinfra::test::set_test_resources_dir(argv[2]);
    else
        tinfra::test::set_test_resources_dir(test_resources_dir);
    return tinfra::cmd::main(argc,argv, test_main);
}

