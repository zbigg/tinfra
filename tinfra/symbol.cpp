#include <string>
#include <iostream>
#include "tinfra/symbol.h"

using namespace std;
namespace tinfra {

symbol::Name2IdMap*      symbol::symbolMapString;
symbol::NamesContainer*  symbol::symbolNames;
int                      symbol::nextFreeSymbolId;

void symbol::initRegistry()
{
	static bool initialized = false;
	if( initialized ) return;

	symbolMapString = new Name2IdMap();
	symbolNames = new NamesContainer();
        const char* null_symbol_name = "null";
	nextFreeSymbolId = 0;	
	initialized = true; 
        
        getIdForName(null_symbol_name);
}

symbol::id_type symbol::getIdForName(const char* name)
{	
	initRegistry();
	// TODO: it must be thread safe
	// synchronize on static mutex
	
	Name2IdMap::iterator i =  symbolMapString->find(name);
	if( i == symbolMapString->end() ) 
	{		
		id_type resultId = nextFreeSymbolId++;
		//cerr << "symbol::register(" << name << ") = " << resultId << endl;
		symbolNames->push_back(name);
		const string& nameInstance = symbolNames->at(resultId);
		(*symbolMapString)[nameInstance.c_str()] = resultId;
		return resultId;
	} 
	else 
	{
		return i->second;
	}
	// UNLOCK
}

symbol	symbol::get(id_type id)
{
	return symbol(id);
}

symbol	symbol::get(const string& name)
{
	return symbol(getIdForName(name.c_str()));
}

symbol	symbol::get(const char* name)
{
	return symbol(getIdForName(name));
}

} // end namespace tinfra


std::ostream& operator <<(std::ostream& dest, tinfra::symbol const& s)
{
    dest << s.c_str();
}
