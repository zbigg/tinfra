#include "adaptable.h"

namespace tinfra {

//
// adaptable
//

adaptable::~adaptable()
{
}

//
// generic_adapter
//

void* generic_adapter::get(std::type_info const& ti)
{
    type_info_wrapper tid(ti);
    adapter_map_t::const_iterator i = adapters.find(tid);
    if( i == adapters.end() ) 
        return 0;
    else
        return i->second;
}

void generic_adapter::add(std::type_info const& ti, void* value)
{
    type_info_wrapper tid(ti);
    adapters[tid] = value;
}

} // end namespace tinfra

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:
