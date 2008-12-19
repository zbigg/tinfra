#ifndef guarded_map_h__
#define guarded_map_h__

#include <map>

#include "guarded_base.h"

template <typename K, typename V>
class guarded_map: public guarded< std::map<K,V> > {
public:
	typedef K key_type;
	typedef V value_type;

        typedef std::map<K,V> delegate_type;
	
	/**
	   Check if specified item exists in map.
	   
	   	@param   key    item searched
		@return  true   item with given key exists in mapping
		
		@pre     contains(key) in (true, false)
		@post    const operation
	*/
	bool contains(key_type const& key) const 
	{
		base_rw_lockable::read_guard g(*this);
		
		typename delegate_type::const_iterator i = this->delegate().find(key);
		if( i == this->delegate().end() ) 
			return false;
		return true;
	}
	
	/**
	   Find by key and return value by reference.
	   
	   	@param   key    item searched
		@param   result where to assign result
		@return  true if found
		
		@pre     contains(key) in (true, false)
		@post    contains(key) == true -> result == get(key)		
	*/
	bool find(key_type const& key, value_type& result) const 
	{
		base_rw_lockable::read_guard g(*this);
		
		typename delegate_type::const_iterator i = this->delegate().find(key);
		if( i == this->delegate().end() ) 
			return false;
		result = i->second;
		return true;
	}
	
	/**
	   Find by key and return value by value.
	   
	   	@param   key    item searched
		@param   default_value what should be returned upon lookup failure
		@return value from found item or default_value
		
		@pre     contains(key) in (true, false)
		@post    const operation
	*/
	value_type get(key_type const& key, value_type const& default_result) const
	{
		// no guard needed: find implements guard
		value_type result;
		if( find(key, result) ) 
			return result;
		else
			return default_result;
	}
	
	/**
	   Put item to map.
	   
	   @param   key    inserted key
	   @param   value  inserted value
	   
	   @pre     contains(key) in (true, false)
	   @post    contains(key) == true
	*/
	void put(key_type const& key, value_type const& value)
	{
		base_rw_lockable::write_guard g(*this);
		
		this->delegate().insert(std::make_pair(key, value));
	}
	
	/**
	   Remove item from map.
	   
	   @param   key    inserted key
	   @return  true if item was really removed, false if it was missing
	   
	   @pre     contains(key) in (true, false)
	   @post    contains(key) == false
	*/
	bool remove(key_type const& key)
	{
		base_rw_lockable::write_guard g(*this);
		
		typename delegate_type::iterator i = this->delegate().find(key);
		if( i == this->delegate().end() )
			return false;
		this->delegate().erase(i);
		return true;
	}
	
	bool empty() const {
		base_rw_lockable::read_guard g(*this);
		
		return this->delegate().empty();
	}
	
	std::size_t size() const {
		base_rw_lockable::read_guard g(*this);
		
		return this->delegate().size();
	}
        
};

#endif // guarded_map_h__

