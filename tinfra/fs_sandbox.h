//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_fs_sandbox_h__
#define __tinfra_fs_sandbox_h__

#include <string>

#include "tinfra/vfs.h"

namespace tinfra {

class fs_sandbox {
public:
    explicit fs_sandbox(tinfra::vfs& fs, std::string const& name = "");
    
    ~fs_sandbox();
    
    fs&         fs() { return _fs; }
    std::string path() { return _path; }
    
    void prepare();
    void cleanup();
    
private:
    void cleanup_nothrow();    
    tinfra::vfs& _fs;
    std::string  _path;
    std::string  _orig_path;
    std::string  _name;
};

} // end namespace tinfra

#endif // 

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

