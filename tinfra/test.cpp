//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "tinfra/test.h"

#include "tinfra/fs.h"
#include "tinfra/path.h"
#include "tinfra/fmt.h"

#include <iostream>

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
static std::string top_srcdir = SRCDIR;
#else
static std::string top_srcdir = ".";
#endif

TempTestLocation::TempTestLocation(std::string const& name)
    : name_(name), orig_pwd_(""), tmp_path_("") 
{
    init();
}

void TempTestLocation::init()
{
    tmp_path_ = path::tmppath();
    fs::mkdir(tmp_path_.c_str());
    if( name_.size() > 0 ) {
        string real_path = path::join(top_srcdir, name_);
        if( !path::exists(real_path) ) {
            throw tinfra::generic_exception(fmt("unable to find test resource %s (%s)") % name_ % real_path);
        }
        string name_in_tmp_ = path::join(tmp_path_, name_);
        fs::recursive_copy(real_path, name_in_tmp_);
    } 
    orig_pwd_ = fs::pwd();
    fs::cd(tmp_path_.c_str());
}
TempTestLocation::~TempTestLocation()
{
    fs::cd(orig_pwd_.c_str());
    if( path::exists(tmp_path_) ) {
        fs::recursive_rm(tmp_path_.c_str());
    }
}

std::string TempTestLocation::getPath() const { 
    if( name_.size() > 0 )
        return name_;
    else
        return ".";
}

void TempTestLocation::setTestResourcesDir(std::string const& x)
{
    top_srcdir = x;
}

void user_wait(const char* prompt)
{
    if( prompt ) {
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

typedef std::vector<std::string> test_name_list;

class test_name_matcher {
    test_name_list& names_;
public:
    test_name_matcher(test_name_list& n): names_(n) {}
    
    bool operator()(const UnitTest::Test* const test) const
    {
        std::string test_name = test->m_details.testName;
        return find(names_.begin(), names_.end(), test_name) != names_.end();
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

