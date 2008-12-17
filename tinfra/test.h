//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_test_h__
#define __tinfra_test_h__

#include "tinfra/tstring.h"
#include "tinfra/fs_sandbox.h"
#include "tinfra/vfs.h"

namespace tinfra {
namespace test {

class test_fs_sandbox: public fs_sandbox {
public:
    explicit test_fs_sandbox(tstring const& name = "");
    explicit test_fs_sandbox(tinfra::vfs& vfs, tstring const& name = "");
    
    ~test_fs_sandbox();
    
private:
    std::string name_;
    std::string orig_pwd_;
};

void set_test_resources_dir(tstring const& d);

void user_wait(tstring const& prompt = "");

} } // end namespace tinfra::test

#endif
