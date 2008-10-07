//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

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
        
        bool operator == (int other) const { return symbolId == other; } 
	bool operator != (int other) const { return symbolId != other; } 
	bool operator <  (int other)  const { return symbolId < other; }
        
        // TODO: add string equivalents
        //bool operator == (int other) const { return symbolId == other; } 
	//bool operator != (int other) const { return symbolId != other; } 
	//bool operator <  (int other)  const { return symbolId < other; }
	
	// getters and cast operators
	
	id_type            id() const      { return symbolId; }
        const char*        c_str() const   { return symbolNames->at(symbolId).c_str(); }
	const std::string& str() const     { return symbolNames->at(symbolId); }
	
	operator const string& () const    { return symbolNames->at(symbolId); }
	operator const char* () const      { return c_str(); }
	operator id_type  () const         { return symbolId; }

        static const int null = 0;
        
	// symbol registry
	static symbol	get(id_type id);
	static symbol	get(const string& name);
	static symbol	get(const char* name);
        static symbol	find(const string& name);
	static symbol	find(const char* name);
	
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

std::ostream& operator <<(std::ostream& dest, symbol const& s);

};



#endif
