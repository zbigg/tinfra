//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "tinfra/fs.h"
#include "tinfra/vfs.h"
#include "tinfra/path.h"
#include "tinfra/test.h"
#include <iostream>

#include <stdexcept>

#include <unittest++/UnitTest++.h>

#include "tinfra/sftp_vfs.h"

SUITE(tinfra_ssh)
{
    TEST(sftp_vfs)
    {
        std::string base_command;
        std::string target;
        base_command = "ssh -s";
        target = "localhost";
        std::auto_ptr<tinfra::vfs> fs;
        if( base_command.size() > 0 ) {
            fs = std::auto_ptr<tinfra::vfs>(tinfra::sftp::create(target, base_command));
            //test_vfs(* fs.get() );
        }        
    }
}

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

