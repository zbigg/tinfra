#ifndef __tinfra_symbol_h_
#define __tinfra_symbol_h_

#include <string>
#include <map>
#include <vector>
#include <ostream>

#include "string.h"

namespace tinfra {
	
class symbol {
public:
	typedef int         id_type;
	typedef std::string string;

        symbol(): symbolId(0) {}
	symbol(const char* s): symbolId(getIdForName(s)) {}
	symbol(const string& s): symbolId(getIdForName(s.c_str())) {}
	
	symbol(id_type id): symbolId(id) {}
	symbol(const symbol& s): symbolId(s.symbolId) {}
	
	// comparision
	bool operator == (const symbol& other) const { return symbolId == other.symbolId; } 
	bool operator != (const symbol& other) const { return symbolId != other.symbolId; } 
	bool operator < (const symbol& other)  const { return symbolId < other.symbolId; }
	
	// getters and cast operators
	
	id_type            getId() const   { return symbolId; }
	const std::string& getName() const { return symbolNames->at(symbolId); } 
        const char*        c_str() const   { return symbolNames->at(symbolId).c_str(); }
	
	operator const string& () const    { return symbolNames->at(symbolId); }
	operator const char* () const      { return c_str(); }
	operator id_type  () const         { return symbolId; }

	// symbol registry
	static symbol	get(id_type id);
	static symbol	get(const string& name);
	static symbol	get(const char* name);
	
private:
	// the only member
	id_type        symbolId;

	// symbol registry
	static id_type getIdForName(const char* name);
	static void    initRegistry();

	struct LessString {
		bool operator() (const char* a, const char* b) const
		{ return ::strcmp(a,b) < 0; }
	};
	typedef std::map<const char*, id_type, LessString> Name2IdMap;
	typedef std::vector<string>                        NamesContainer;
	
	static Name2IdMap*     symbolMapString;
	static NamesContainer* symbolNames;

	static int             nextFreeSymbolId;
};


};

std::ostream& operator <<(std::ostream& dest, tinfra::symbol const& s);

#endif
