//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_sftp_vfs_h__
#define __tinfra_sftp_vfs_h__

#include "tinfra/vfs.h"

namespace tinfra {
namespace sftp {
using std::auto_ptr;
using tinfra::vfs;

auto_ptr<vfs> create(std::string const& sftp_subsystem_command);    
auto_ptr<vfs> create(std::string const& target, std::string const& sftp_subsystem_command);

}} // end namespace tinfra::sftp

#endif // __tinfra__sftp_vfs_h__
