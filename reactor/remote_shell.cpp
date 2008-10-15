//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "tinfra/symbol.h"

namespace remote_shell { namespace S {
    tinfra::symbol command("command");
    tinfra::symbol environment("environment");
    
    tinfra::symbol stream_id("stream_id");
    tinfra::symbol status("status");
    tinfra::symbol what("what");
    tinfra::symbol data("data");
} } // end namespace remote_shell::S

