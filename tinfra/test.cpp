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

test_fs_sandbox::test_fs_sandbox(std::string const& name):
	fs_sandbox(tinfra::local_fs()),
	_name(name) 
{
    if( _name.size() > 0 ) {
        string real_path = path::join(top_srcdir, name_);
        if( !fs::exists(real_path) ) {
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
    if( fs::exists(tmp_path_) ) {
        fs::recursive_rm(tmp_path_.c_str());
    }
}

test_fs_sandbox::~test_fs_sandbox()
{
}

void set_test_resources_dir(std::string const& x)
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

} } // end namespace tinfra::test

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

