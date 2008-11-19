#ifndef __tinfra_wx_action_h__
#define __tinfra_wx_action_h__

#include <string>
#include <map>

struct ActionInfo {
    int id;
    std::string name;
    std::string label;
    std::string icon_resource;
    std::string shortcut;
};

class ActionManager {
public:
    static ActionManager* getInstance();

    ActionInfo* registerAction(int id,
                               std::string const& name, 
                               std::string const& label, 
                               std::string const& icon_resource = "",
                               std::string const& shortcut = "");

    ActionInfo* registerAction(std::string const& name, 
                               std::string const& label, 
                               std::string const& icon_resource = "",
                               std::string const& shortcut = "");
    
    ActionInfo* getActionInfo(int id);
    ActionInfo* getActionInfo(std::string const& name);
protected:
    ~ActionManager();
    ActionManager();

private:
    typedef std::map<std::string, ActionInfo*> ActionNameMap;
    typedef std::map<int, ActionInfo*>         ActionIdMap;

    ActionNameMap byNames;
    ActionIdMap   byIds;
};

#define AM_ACTION(id, name, label, icon_resource, shortcut) ActionManager::getInstance()->registerAction(id, name, label, icon_resource, shortcut)
#define AM_ACTION_AUTO(name, label, icon_resource, shortcut) ActionManager::getInstance()->registerAction(name, label, icon_resource, shortcut)

#endif //__tinfra_wx_action_h__
