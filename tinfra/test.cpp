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
} } // end namespace tinfra::test

