#ifndef tinfra_aio_connection_scope_h_included
#define tinfra_aio_connection_scope_h_included

namespace tinfra { 
namespace aio {
    
struct connection_scope: public scope {
    struct connection_aio_dispatcher {
        connection_scope& parent;
    };
    
    // dispatch READ  ev to parser_feeder
    // dispatch WRITE ev to sink
    // dispatch remove/failure as EOL events for whole scope
    connection_aio_dispatcher  aio_listener;
    
    buffered_aio_parser_feeder parser_feeder;
    buffered_aio_sink          sink;
};
    
} } // end namespace tinfra::aio

#endif // tinfra_aio_connection_scope_h_included


