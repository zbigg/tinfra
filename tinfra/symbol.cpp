//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "tinfra/symbol.h"
#include "tinfra/mutex.h"
#include "tinfra/guard.h"

#include <string>
#include <iostream>
#include <list>
#include <map>
#include <vector>

namespace tinfra {

class symbol_registry {
public:
    typedef symbol::id_type id_type;
    
    symbol_registry()
        : next_symbol_id_(0)
    {        
        get_id_for_name("null");
    }
    
    id_type get_id_for_name(tstring const& name)
    {
        tinfra::guard instance_guard(instance_lock_);
        
        name_to_id_mapping_t::const_iterator i =  name_map_.find(name);
	if( i == name_map_.end() ) 
	{		
		id_type result_id = next_symbol_id_++;
		name_storage_t::const_iterator istr = name_storage_.insert(name_storage_.end(), name.str());
                
                tstring name_allocated(*istr);
		name_index_.push_back(istr);                
		name_map_[name_allocated] = result_id;
		return result_id;
	} 
	else 
	{
		return i->second;
	}
    }
    
    id_type find_no_create(tstring const& name)
    {
        tinfra::guard instance_guard(instance_lock_);
        name_to_id_mapping_t::const_iterator i =  name_map_.find(name);
        if( i == name_map_.end() ) {
            return 0;             
        } else {
            return i->second;
        }
    }
    std::string const& name_for_id(id_type const& id)
    {
        tinfra::guard instance_guard(instance_lock_);
        
        return * (name_index_[id] );
    }
private:
    typedef std::map<tstring, id_type> name_to_id_mapping_t;
    typedef std::list<std::string>     name_storage_t;
    typedef std::vector<name_storage_t::const_iterator> name_index_t;
    
    
    id_type               next_symbol_id_;
    
    name_to_id_mapping_t name_map_;
    name_index_t         name_index_;
    name_storage_t       name_storage_;
    
    tinfra::mutex instance_lock_;
};

symbol_registry& global_register() {
    static symbol_registry instance;
    return instance;
}

symbol::id_type symbol::get_id_for_name(tstring const& str)
{    
    return global_register().get_id_for_name(str);
}

std::string const& symbol::name_for_id(symbol::id_type const& id)
{
    return global_register().name_for_id(id);
}

const symbol symbol::null = symbol(0);

symbol	symbol::get(id_type id)
{
	return symbol(id);
}

symbol	symbol::get(const tstring& name)
{
	return symbol(get_id_for_name(name));
}

symbol	symbol::find(tstring const& name)
{
    id_type id = global_register().find_no_create(name);
    return symbol(id);
}

std::ostream& operator <<(std::ostream& dest, symbol const& s)
{
    return dest << s.str();
}

} // end namespace tinfra

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

