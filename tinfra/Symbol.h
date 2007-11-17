#ifndef __tinfra_symbol_h_
#define __tinfra_symbol_h_

#include <string>
#include <map>
#include <vector>

#include "string.h"
namespace tinfra {
	
class Symbol {
public:
	typedef int         id_type;
	typedef std::string string;

        Symbol(): symbolId(0) {}
	Symbol(const char* s): symbolId(getIdForName(s)) {}
	Symbol(const string& s): symbolId(getIdForName(s.c_str())) {}
	
	Symbol(id_type id): symbolId(id) {}
	Symbol(const Symbol& s): symbolId(s.symbolId) {}
	
	// comparision
	bool operator == (const Symbol& other) const { return symbolId == other.symbolId; } 
	bool operator != (const Symbol& other) const { return symbolId != other.symbolId; } 
	bool operator < (const Symbol& other)  const { return symbolId < other.symbolId; }
	
	// getters and cast operators
	
	id_type            getId() const   { return symbolId; }
	const std::string& getName() const { return symbolNames->at(symbolId); } 
        const char*        c_str() const   { return symbolNames->at(symbolId).c_str(); }
	
	operator const string& () const    { return symbolNames->at(symbolId); }
	operator const char* () const      { return c_str(); }
	operator id_type  () const         { return symbolId; }

	// Symbol registry
	static Symbol	get(id_type id);
	static Symbol	get(const string& name);
	static Symbol	get(const char* name);
	
private:
	// the only member
	id_type        symbolId;

	// Symbol registry
	static id_type getIdForName(const char* name);
	static void    initRegistry();

	struct LessString {
		bool operator() (const char* a, const char* b)
		{ return ::strcmp(a,b) < 0; }
	};
	typedef std::map<const char*, id_type, LessString> Name2IdMap;
	typedef std::vector<string>                        NamesContainer;
	
	static Name2IdMap*     symbolMapString;
	static NamesContainer* symbolNames;

	static int             nextFreeSymbolId;
};


};

#endif
