#include <string>
#include <iostream>
#include "Symbol.h"

using namespace std;
namespace tinfra {

Symbol::Name2IdMap*      Symbol::symbolMapString;
Symbol::NamesContainer*  Symbol::symbolNames;
int                      Symbol::nextFreeSymbolId;

void Symbol::initRegistry()
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

Symbol::id_type Symbol::getIdForName(const char* name)
{	
	initRegistry();
	// TODO: it must be thread safe
	// synchronize on static mutex
	
	Name2IdMap::iterator i =  symbolMapString->find(name);
	if( i == symbolMapString->end() ) 
	{		
		id_type resultId = nextFreeSymbolId++;
		//cerr << "Symbol::register(" << name << ") = " << resultId << endl;
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

Symbol	Symbol::get(id_type id)
{
	return Symbol(id);
}

Symbol	Symbol::get(const string& name)
{
	return Symbol(getIdForName(name.c_str()));
}

Symbol	Symbol::get(const char* name)
{
	return Symbol(getIdForName(name));
}

};
