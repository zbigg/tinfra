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
    struct dummy_visitor: tinfra::fs::file_list_visitor {
        virtual void accept(const tinfra::tstring&)
        {
        }
    };
    TEST(sftp_vfs)
    {
        //std::string base_command;
        //std::string target;
        //base_command = "plink.exe";
        //target = "localhost";
        std::auto_ptr<tinfra::vfs> fs;
        fs = std::auto_ptr<tinfra::vfs>(tinfra::sftp::create("plink -s localhost sftp"));
        
        fs->stat("/");
        
        dummy_visitor v;
        fs->list_files("/", v);

    }
}

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

