//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_protocol_message_raw_h__
#define __tinfra_protocol_message_raw_h__

#include <string>
#include "tinfra/io/stream.h"

#include "protocols.h"

namespace message_raw {

class ProtocolHandler: public ::ProtocolHandler {
public:
    // accept bytes from ProtocolManager and maybe assemble complete message using 'message_raw' protocol
    // assembled messages are passed to accept_message
    virtual int  accept_bytes(const char* data, int length, tinfra::io::stream* channel);
    
    virtual void accept_message(const std::string& message, tinfra::io::stream* channel) = 0;
    
    /// send binary message
    /// 
    /// send message using 'message_raw' protocol
    virtual void send_message(tinfra::io::stream* channel, const std::string& message);
    
};

} // end namespace message_raw

#endif __tinfra_protocol_message_raw_h__

