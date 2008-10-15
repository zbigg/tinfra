//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __remote_shell_protocol_h__
#define __remote_shell_protocol_h__

#include <string>
#include <vector>

#include "tinfra/symbol.h"

#define TINFRA_DECLARE_STRUCT template <typename F> void apply(F& field) const
#define FIELD(a) field(S::a, a)

namespace remote_shell {
    namespace S {
        extern tinfra::symbol command;
        extern tinfra::symbol environment;
        
        extern tinfra::symbol stream_id;
        extern tinfra::symbol what;
        extern tinfra::symbol status;
        extern tinfra::symbol data;
    }
    
    enum status_code {
        OK = 0,
        FAILED = 1,
        FINISHED = 2
    };
    
    struct request {
        std::vector<std::string> command;
        std::vector<std::string> environment;
        
        TINFRA_DECLARE_STRUCT {
            FIELD(command);
            FIELD(environment);
        }
    };
    
    struct stream_data {
        int           stream_id;        
        int           status;
        int           what;
        std::string   data;
        
        TINFRA_DECLARE_STRUCT {
            FIELD(stream_id);
            FIELD(what);
            FIELD(status);
            FIELD(data);
        }
    };

} } // end namespace remote_shell::S

#endif //__remote_shell_protocol_h__


