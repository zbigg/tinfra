//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "tinfra/platform.h"

#include "tinfra/test.h"

#include "tinfra/fs.h"
#include "tinfra/path.h"
#include "tinfra/fmt.h"
#include "tinfra/trace.h"

#include <iostream>
#include <algorithm>

#include <unittest++/UnitTest++.h>
#include <unittest++/TestReporter.h>
#include <unittest++/TestDetails.h>
#include <unittest++/TestResults.h>
#include <unittest++/TestReporter.h>

#include <stdio.h> // screw cxxx headers because they are incompatible
#include <stdarg.h>

#ifdef _WIN32
#include <windows.h>
#endif

namespace tinfra {
namespace test {

using std::string;
using std::cout;
using std::cin;
using std::endl;
    
#ifdef SRCDIR
    #define DEFAULT_TOP_SRCDIR SRCDIR
#else
    #define DEFAULT_TOP_SRCDIR "."
#endif

static std::string top_srcdir = DEFAULT_TOP_SRCDIR;

TINFRA_MODULE_TRACER(tinfra_test_fs_sandbox);

test_fs_sandbox::test_fs_sandbox(tstring const& name):
	fs_sandbox(tinfra::local_fs()),
	name_(name.str()) 
{
    TINFRA_USE_TRACER(tinfra_test_fs_sandbox);
    if( name_.size() > 0 ) {
        string real_path = path::join(top_srcdir, name_);
        if( !fs::exists(real_path) ) {
            throw tinfra::generic_exception(fmt("unable to find test resource %s (%s)") % name_ % real_path);
        }
        
        fs::recursive_copy(real_path, fs_sandbox::path());
    } 
    orig_pwd_ = fs::pwd();
    fs::cd(fs_sandbox::path());
    TINFRA_TRACE_MSG(fmt("entering sandbox pwd='%s'") % fs_sandbox::path());
}
test_fs_sandbox::~test_fs_sandbox()
{
    TINFRA_USE_TRACER(tinfra_test_fs_sandbox);
    TINFRA_TRACE_MSG(fmt("leaving sandbox pwd='%s'") % orig_pwd_);
    fs::cd(orig_pwd_);    
}

void set_test_resources_dir(tstring const& x)
{
    top_srcdir.assign(x.data(), x.size());
}

void user_wait(tstring const& prompt)
{
    if( prompt.size() != 0 ) {
        cout << prompt << " ";
    }
    cout << "(waiting for enter)";
    cout.flush();
    std::string s;
    std::getline(cin, s);
}

static void out(const char* message, ...)
{
    va_list ap;
    va_start(ap, message);
#ifdef _WIN32
    char buf[2048];
    vsprintf(buf,message, ap);
    //printf("%s",buf);
    std::cout << buf;
    std::cout.flush();
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

typedef std::vector<std::string> test_name_list;

class test_name_matcher {
    test_name_list& names_;
public:
    test_name_matcher(test_name_list& n): names_(n) {}
    
    static bool matches(tstring const& mask, tstring const& str)
    {
        if( mask == str )
            return true;
        if( mask.size() > 0 && mask[mask.size()-1] == '*') {
            size_t const_part_len = mask.size()-1;
            
            if( str.size() >= const_part_len &&
                str.substr(0,const_part_len) == mask.substr(0, const_part_len) )
                return true;
        }
        return false;
    }
    bool operator()(const UnitTest::Test* const test) const
    {
        std::string test_name      = test->m_details.testName;
        std::string full_test_name = fmt("%s::%s") % test->m_details.suiteName % test_name;
        
        for( test_name_list::const_iterator i = names_.begin(); i != names_.end(); ++i )
        {
            std::string const& mask = *i;
            if( matches(mask, test_name) ) 
                return true;
            if( matches(mask, full_test_name) ) 
                return true;
        }
        return false;
    }
};

int test_main(int argc, char** argv)
{
    TinfraTestReporter reporter;
    UnitTest::TestRunner runner(reporter);
    test_name_list test_names;
    for(int i = 1; i < argc; ++i ) {
        if( strncmp(argv[i],"-",1) != 0 ) 
            test_names.push_back(argv[i]);
    }
    if( ! test_names.empty() ) {        
        test_name_matcher predicate(test_names);        
        return runner.RunTestsIf(UnitTest::Test::GetTestList(), 0, predicate, 0);
    } else {
        return runner.RunTestsIf(UnitTest::Test::GetTestList(), 0, UnitTest::True(), 0);
    }
}

} } // end namespace tinfra::test

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++

