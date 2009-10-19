//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "tinfra/fs_sandbox.h"

#include "tinfra/vfs.h"

#include "tinfra/path.h"
#include "tinfra/fmt.h"
#include "tinfra/cmd.h"

#include <iostream>

namespace tinfra {

using std::string;
using std::cout;
using std::cin;
using std::endl;
 
fs_sandbox::fs_sandbox(tinfra::vfs& fs, tstring const& path): 
	_fs(fs),
	_orig_path(path.str())
{
    prepare();
}

fs_sandbox::~fs_sandbox()
{
    cleanup_nothrow();
}

void fs_sandbox::prepare()
{
    cleanup();
    _path = _orig_path;
    if( _path.size() == 0 ) {
        _path = path::tmppath();
    }
    ensure_dir_exists(_fs, _path);
}

void fs_sandbox::cleanup()
{
    if( _path.size() == 0 )
        return;
    string tmp_path = _path;
    
    _path = "";
    _fs.recursive_rm(tmp_path);
}

void fs_sandbox::cleanup_nothrow()
{
    std::string tmp_path = _path;
    try {
        cleanup();
    } catch( std::exception& e ) {
        get_app().silent_exception(fmt("unable to cleanup sandbox at '%s': %s") % tmp_path % e.what());;
    }
}

} //  end namespace tinfra

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

