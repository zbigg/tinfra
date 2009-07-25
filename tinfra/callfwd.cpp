#include "callfwd.h"

namespace callfwd {
namespace detail {
    
namespace S {
    tinfra::symbol p1("p1");
    tinfra::symbol p2("p2");
    tinfra::symbol p3("p3");
    tinfra::symbol message_id("message_id");
    tinfra::symbol arguments("arguments");
}

const message_serial_id message0::serial_id("0");


// empty detructors for some base virtuals
dynamic_any_container::~dynamic_any_container()
{
}

partial_invoker_base::~partial_invoker_base() 
{
}

} // end namespace callfwd::detail 

} // end namespace callfwd
