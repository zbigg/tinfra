//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include <stdexcept>
#include <string>

#include "tinfra/io/stream.h"
#include "tinfra/symbol.h"
#include "network_serializer.h"

#include "message_raw.h"

namespace message_raw {

tinfra::symbol null_symbol;
    
int  ProtocolHandler::accept_bytes(const char* data, int length, tinfra::io::stream* channel)
{        
    
    std::string message;
    try {
        network_serializer::reader r(data, length);
        r(null_symbol, message);
    } catch( tinfra::would_block& wb) {
        return 0;
    }
    
    // TODO process message
    accept_message(message, channel);
    return r.readed;
}
    
void ProtocolHandler::send_message(tinfra::io::stream* channel, const std::string& message)
{
    std::string buffer;
    
    buffer.reserve( message.size() + 4);
    {        
        network_serializer::writer w(buffer);        
        w(null_symbol, message);
    }
    
    channel->write(buffer.data(), buffer.size());
    
}

} // end namespace message_raw


