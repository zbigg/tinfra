#include <allpch.h>

#include "action.h"

ActionManager* ActionManager::getInstance()
{
    static ActionManager instance;
    return &instance;
}

ActionManager::ActionManager()
{
}

ActionManager::~ActionManager()
{
    for( ActionIdMap::const_iterator i = byIds.begin(); i != byIds.end(); ++i ) 
    {
        delete i->second;
    }
}


ActionInfo* ActionManager::registerAction(int id,
                         std::string const& name, 
                         std::string const& label, 
                         std::string const& icon_resource,
                         std::string const& shortcut)
{
    ActionInfo* ai = new ActionInfo();
    ai->id = id;
    ai->name = name;
    ai->label = label;
    ai->icon_resource = icon_resource;
    ai->shortcut = shortcut;
    
    byIds[id] = ai;
    byNames[name] = ai;
    return ai;
}

ActionInfo* ActionManager::registerAction(std::string const& name, 
                         std::string const& label, 
                         std::string const& icon_resource,
                         std::string const& shortcut)
{
    return registerAction(wxNewId(), name, label, icon_resource, shortcut);
}
    
ActionInfo* ActionManager::getActionInfo(int id)
{
    ActionIdMap::const_iterator a = byIds.find(id);
    if( a != byIds.end() )
        return a->second;
    else
        return 0;
}

ActionInfo* ActionManager::getActionInfo(std::string const& name)
{
    ActionNameMap::const_iterator a = byNames.find(name);
    if( a != byNames.end() )
        return a->second;
    else
        return 0;
}