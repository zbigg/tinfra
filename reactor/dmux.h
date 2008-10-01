//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_protocols_dmux_h__
#define __tinfra_protocols_dmux_h__

#include <string>

#include "tinfra/io/stream.h"
#include "tinfra/symbol.h"

#define TINFRA_DECLARE_STRUCT template <typename F> void apply(F& field) const
#define FIELD(a) field(S::a, a)

namespace dmux {
    
    namespace S {
        extern tinfra::symbol message_type;
        
        extern tinfra::symbol request_id;
        extern tinfra::symbol last_response_id;
        extern tinfra::symbol status;
        
        extern tinfra::symbol address;
        
        extern tinfra::symbol connection_id;
        extern tinfra::symbol connection_status;
        
        extern tinfra::symbol data;
    }
    
    enum message_type_t {
        INFO,
        CONNECT,
        EVENT
    };
    
    struct message_header {        
        unsigned short message_type;
        unsigned int   request_id;
        unsigned int   last_response_id;
        unsigned int   status;
        
        TINFRA_DECLARE_STRUCT {
            FIELD(message_type);
            FIELD(request_id);
            FIELD(last_response_id);
            FIELD(status);
        }
    };

    struct connect {
        std::string address;
        
        TINFRA_DECLARE_STRUCT {
            FIELD(address);
        }
    };
    
    enum connection_status_bits {
        DATA        = 1,
        ESTABLISHED = 2,
        CLOSED      = 4,
        FAILED      = 8
    };
    struct event {
        unsigned int  connection_id;
        unsigned int  connection_status;
        std::string   data;
        
        TINFRA_DECLARE_STRUCT {
            FIELD(connection_id);
            FIELD(connection_status);
            FIELD(data);
        }
    };

} // end namespace dmux

#endif __tinfra_protocols_dmux_h__
