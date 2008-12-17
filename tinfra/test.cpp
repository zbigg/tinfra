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
#include "tinfra/trace.h"

#include <iostream>
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

} } // end namespace tinfra::test

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

