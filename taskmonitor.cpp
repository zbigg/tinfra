#include <string>
#include <vector>
#include <iostream>
#include <ostream>
#include <typeinfo>
#include <exception>
#include <fstream>

#include <tinfra/Symbol.h>
using namespace std;

#include <tinfra/tinfra.h>
#include <tinfra/tinfra_lex.h>
#include <tinfra/xml.h>
#include <tinfra/xml/XMLStream.h>

using tinfra::Symbol;
using tinfra::ManagedType;

// 
// classes 
//

///
// Address
///

enum yesno {
    no, yes
};

namespace tinfra {
template<> 
struct LexicalInterpreter<yesno> {
    static void to_string(const yesno& v, std::ostream& dest) {
        dest << ( (v == no) ? "no" : "yes");
    }
    static void to_string(const yesno& v, std::string& dest) {
        dest = ( (v == no) ? "no" : "yes");
    }
    static void from_string(const char* v, yesno& dest) {
        dest = (v[0] == 'y') ? yes : no;
    }
};
}
namespace S {
    // symbols
    extern Symbol active;
    extern Symbol closed;
    extern Symbol created;
    extern Symbol deadline;
    extern Symbol description;
    extern Symbol end;
    extern Symbol fileversion;
    extern Symbol fullname;
    extern Symbol group;
    extern Symbol hide;
    extern Symbol id;
    extern Symbol master_task;
    extern Symbol master_tasks;
    extern Symbol measure_id;
    extern Symbol members;
    extern Symbol members_data;
    extern Symbol name;
    extern Symbol persistent;
    extern Symbol process_measures;
    extern Symbol progress;
    extern Symbol project;
    extern Symbol project_phases;
    extern Symbol projects;
    extern Symbol projectphase_id;
    extern Symbol shortname;
    extern Symbol start;
    extern Symbol started;
    extern Symbol state;
    extern Symbol time_load;
    extern Symbol tasks;
    extern Symbol version;
}

// Member definition
struct Member {
    yesno   active;
    string  fullname;
    Symbol  id;
    string  shortname;
    
    template <typename F>
    void apply(F& f) const
    {
        f(S::active,    active);
        f(S::fullname,   fullname);
        f(S::id,  id);
        f(S::shortname,   shortname);
    }
};

// Group definition

struct Group {
    string fullname;
    vector<Member> members;
    
    template <typename F>
    void apply(F& f) const
    {
        f(S::fullname,  fullname);
        f(S::members,   members);
    }
};

struct Project {
    yesno    active;
    string   fullname;
    Symbol   id;
    
    template <typename F>
    void apply(F& f) const
    {
        f(S::active,  active);
        f(S::fullname,   fullname);
        f(S::id,  id);
    }
};

struct ProcessMeasure {
    Symbol   id;
    string   name;
        
    template <typename F>
    void apply(F& f) const
    {
        f(S::id,  id);
        f(S::name,  name);
    }
};

struct ProjectPhase {
    Symbol   id;
    string   name;
    
    template <typename F>
    void apply(F& f) const
    {
        f(S::id,  id);
        f(S::name,  name);
    }
};

struct MasterTask {
    Symbol id;
    Symbol project;
    string state;
    yesno  active;
    yesno  persistent;
    string description;
    
    template <typename F>
    void apply(F& f) const
    {
        f(S::id,  id);
        f(S::project,  project);
        f(S::state,  state);
        f(S::active,  active);
        f(S::persistent,  persistent);
        f(S::description, description);
    }
};

struct timestamp {
    long timestamp;
};

struct TimeLoadEntry {
    timestamp start;
    timestamp end;
    
    template <typename F>
    void apply(F& f) const
    {
        f(S::start,  start);
        f(S::end,    end);
    }
};

enum taskstate {
    prepared, active, closed
};

struct Task {
    string      name;
    Symbol      project;
    Symbol      master_task;
    timestamp   created;
    timestamp   started;
    int    progress;
    taskstate   state;
    timestamp   closed;
    timestamp   deadline;
    yesno       hide;
    Symbol      measure_id;
    Symbol      projectphase_id;
    string      description;
    vector<TimeLoadEntry> time_load;
    
    template <typename F>
    void apply(F& f) const
    {
        f(S::name,  name);
        f(S::project,  project);
        f(S::master_task,  master_task);
        f(S::created,  created);
        f(S::started,  started);
        f(S::progress,  progress);
        f(S::state,  state);
        f(S::closed,  closed);
        f(S::deadline, deadline);
        f(S::hide, hide);
        f(S::measure_id, measure_id);
        f(S::projectphase_id, projectphase_id);
        f(S::description, description);
        f(S::time_load, time_load);
    }
};

struct MemberData {
    yesno    active;
    Symbol   id;
    vector<Task>        tasks;
    
    template <typename F>
    void apply(F& f) const
    {
        f(S::active,  active);
        f(S::id,  id);
        f(S::tasks,  tasks);
    }
};
struct FileVersion {
    int version;
    
    template <typename F>
    void apply(F& f) const
    {
        f(S::version, version);
    }
};

struct TaskMonitor {
    FileVersion     fileversion;
    
    Group           group;
    
    vector<Project> projects;
    vector<ProcessMeasure> process_measures;
    
    vector<ProjectPhase>   project_phases;
    
    vector<MasterTask>     master_tasks;
    vector<MemberData>     members_data;
    
    template <typename F>
    void apply(F& f) const
    {
        f(S::group, group);
        f(S::fileversion,  fileversion);
        f(S::projects,  projects);
        f(S::process_measures,  process_measures);
        f(S::project_phases,  project_phases);
        f(S::master_tasks,  master_tasks);
        f(S::members_data,  members_data);
    }
};

namespace tinfra {
	template<> struct TypeTraits<TaskMonitor>: public ManagedStruct<TaskMonitor> {};
	template<> struct TypeTraits<Group>: public ManagedStruct<Group> {};
        template<> struct TypeTraits<Member>: public ManagedStruct<Member> {};
        template<> struct TypeTraits<MemberData>: public ManagedStruct<MemberData> {};
        template<> struct TypeTraits<Project>: public ManagedStruct<Project> {};
        template<> struct TypeTraits<ProjectPhase>: public ManagedStruct<ProjectPhase> {};
        template<> struct TypeTraits<ProcessMeasure>: public ManagedStruct<ProcessMeasure> {};
        template<> struct TypeTraits<MasterTask>: public ManagedStruct<MasterTask> {};
        template<> struct TypeTraits<FileVersion>: public ManagedStruct<FileVersion> {};
        template<> struct TypeTraits<Task>: public ManagedStruct<Task> {};
        template<> struct TypeTraits<TimeLoadEntry>: public ManagedStruct<TimeLoadEntry> {};
        template<> struct TypeTraits<timestamp>: public Fundamental<timestamp> {};
        template<> struct TypeTraits<taskstate>: public Fundamental<taskstate> {};

	template<typename T> struct TypeTraits<vector<T> >: public STLContainer<vector<T> > {};
}

ostream& operator << (ostream& o, const Symbol& s)  { return o << s.getName().c_str(); }
ostream& operator << (ostream& o, const timestamp& s)  { return o << s.timestamp; }
ostream& operator << (ostream& o, const taskstate& s)  { 
    return o << ( s == prepared ? "prepared" :
                  s == active   ? "active":
                  s == closed   ? "closed" : "<error>"); 
}
namespace S {
    // symbols
    Symbol active = "active";
    Symbol closed = "closed";;
    Symbol created = "created";;
    Symbol deadline = "deadline";;
    Symbol description = "description";;
    Symbol end = "end";;
    Symbol fileversion = "fileversion";
    Symbol fullname = "fullname";;
    Symbol group = "group";
    Symbol hide = "hide";
    Symbol id = "id";
    Symbol master_task = "master_task";
    Symbol master_tasks = "master_tasks";
    Symbol measure_id = "measure_id";
    Symbol members = "members";
    Symbol members_data = "members_data";
    Symbol name = "name";
    Symbol persistent = "persistent";
    Symbol process_measures = "process_measures";
    Symbol progress = "progress";
    Symbol project = "project";
    Symbol project_phases = "project_phases";
    Symbol projects = "projects";
    Symbol projectphase_id = "projectphase_id";
    Symbol shortname = "shortname";
    Symbol start = "start";
    Symbol started = "started";
    Symbol state = "state";
    Symbol time_load = "time_load";
    Symbol tasks = "tasks";
    Symbol version = "version";
}

void initialize_xml_mapping(tinfra::xml::XMLSymbolMapping& xml_mapping)
{
    xml_mapping.map_class_by_traits<TaskMonitor>(Symbol("taskmonitor"));
    xml_mapping.map_class_by_traits<ProcessMeasure>(Symbol("process-measure"));
    xml_mapping.map_class_by_traits<ProjectPhase>(Symbol("project-phase"));
    xml_mapping.map_class_by_traits<Task>(Symbol("task"));
    xml_mapping.map_class_by_traits<MasterTask>(Symbol("master-task"));
    xml_mapping.map_class_by_traits<TimeLoadEntry>(Symbol("time-load-entry"));
    xml_mapping.map_class_by_traits<MemberData>(Symbol("member-data"));
    xml_mapping.map_class_by_traits<Project>(Symbol("project"));
    xml_mapping.map_class_by_traits<Member>(Symbol("member"));
        
    xml_mapping.map_field(S::process_measures,Symbol("process-measures"));
    xml_mapping.map_field(S::project_phases,Symbol("project-phases"));
    xml_mapping.map_field(S::master_tasks,Symbol("master-tasks"));
    xml_mapping.map_field(S::master_task,Symbol("master-task"));
    xml_mapping.map_field(S::projectphase_id,Symbol("projectphase-id"));
    xml_mapping.map_field(S::measure_id,Symbol("measure-id"));
    xml_mapping.map_field(S::members_data,Symbol("members-data"));
    xml_mapping.map_field(S::time_load,Symbol("time-load"));    
}

void test1()
{
    TaskMonitor tm;
    tm.fileversion.version = 9;
    tm.group.fullname = "Group1";
    
    tinfra::xml::XMLSymbolMapping xml_symbol_mapping;
    initialize_xml_mapping(xml_symbol_mapping);
    
    tm.process_measures.push_back(tinfra::construct<ProcessMeasure>()
                                                           (S::id, Symbol("req-1"))
                                                           (S::name, string("XXX")));
    tm.project_phases.push_back(tinfra::construct<ProjectPhase>()
                                                           (S::id, Symbol("m1"))
                                                           (S::name, string("Test")));
    tinfra::xml::FileXMLOutputStream xml_out(* cout.rdbuf());
    tinfra::xml::dump(xml_out, Symbol("taskmonitor"), tm, xml_symbol_mapping);
}

void test2()
{
    TaskMonitor tm;
    
    tinfra::xml::XMLSymbolMapping xml_symbol_mapping;
    initialize_xml_mapping(xml_symbol_mapping);
    {
        
        
        tinfra::xml::XMLMemoryStream xml_in;
        tinfra::xml::read_xml("taskmonitor2.xml",xml_in);
        tinfra::xml::load<TaskMonitor>(xml_in, Symbol("taskmonitor"), tm, xml_symbol_mapping);
        cout << "readed!" << endl;
    }
    {
        tinfra::xml::FileXMLOutputStream xml_out(* cout.rdbuf());
        tinfra::xml::dump(xml_out, Symbol("taskmonitor"), tm, xml_symbol_mapping);
        cout << "written!" << endl;
    }
    cout << "bye!" << endl;
}


int main2()
{
    //test1();
    test2();
    return 0;
}

int main()
{
    try {
        main2();
    } catch( std::exception& e) {
        cerr << "fatal error: " << e.what() << endl;
    }
}
