//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef tinfra_symbol_h_included
#define tinfra_symbol_h_included

#include "tinfra/tstring.h"

#include <string>
#include <ostream>

namespace tinfra {
	
class symbol {
public:
	typedef int         id_type;
	typedef std::string string;

        symbol(): id_(0) {}
	symbol(tstring const& s): id_(get_id_for_name(s)) {}
	
	explicit symbol(id_type id): id_(id) {}
	symbol(const symbol& s): id_(s.id_) {}
	
	// comparision
	bool operator == (const symbol& other) const { return id_ == other.id_; } 
	bool operator != (const symbol& other) const { return id_ != other.id_; } 
	bool operator <  (const symbol& other)  const { return id_ < other.id_; }
        
        bool operator == (id_type other) const { return id_ == other; } 
	bool operator != (id_type other) const { return id_ != other; } 
	bool operator <  (id_type other)  const { return id_ < other; }
        
	
	// getters and cast operators
	
	id_type            id() const      { return id_; }
        const char*        c_str() const   { return name_for_id(id_).c_str(); }
	const std::string& str() const     { return name_for_id(id_); }
	
	operator const string& () const    { return str(); }
	operator const char* () const      { return c_str(); }
	operator id_type  () const         { return id_; }

	static const symbol null;
        
	// symbol registry
	static symbol	get(id_type id);
	static symbol	get(const tstring& name);
	static symbol	find(const tstring& name);
	
private:
	// the only member
	id_type        id_;

	// symbol registry access
	static id_type get_id_for_name(tstring const& name);
	static std::string const& name_for_id(id_type const& id);	
};

std::ostream& operator <<(std::ostream& dest, symbol const& s);

}

#endif // tinfra_symbol_h_included


