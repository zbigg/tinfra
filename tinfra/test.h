//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_test_h__
#define __tinfra_test_h__

#include <string>

namespace tinfra {
namespace test {

class test_fs_sandbox: public fs_sandbox {
public:
    explicit test_fs_sandbox(std::string const& name = "");
    explicit test_fs_sandbox(tinfra::vfs& vfs, std::string const& name = "");
    
    ~test_fs_sandbox();
    
private:
    std::string name_;    
};

void set_test_resources_dir(std::string const& d);

void user_wait(const char* prompt = 0);

} } // end namespace tinfra::test

#endif
