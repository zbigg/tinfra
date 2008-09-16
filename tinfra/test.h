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

class TempTestLocation {
public:
    explicit TempTestLocation(std::string const& name = "");
    ~TempTestLocation();
    
    std::string getPath() const;
        
    static void setTestResourcesDir(std::string const& x);
private:
    void init();
    std::string name_;
    std::string orig_pwd_;
    std::string tmp_path_;
};

void user_wait(const char* prompt = 0);

} } // end namespace tinfra::test

#endif
