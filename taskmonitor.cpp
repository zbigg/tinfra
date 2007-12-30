#include <string>
#include <vector>
#include <iostream>
#include <ostream>
#include <typeinfo>
#include <exception>
#include <fstream>

#include <tinfra/symbol.h>
using namespace std;

#include <tinfra/tinfra.h>
#include <tinfra/tinfra_lex.h>
#include <tinfra/xml.h>
#include <tinfra/xml/XMLStream.h>

using tinfra::symbol;
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
    extern symbol active;
    extern symbol closed;
    extern symbol created;
    extern symbol deadline;
    extern symbol description;
    extern symbol end;
    extern symbol fileversion;
    extern symbol fullname;
    extern symbol group;
    extern symbol hide;
    extern symbol id;
    extern symbol master_task;
    extern symbol master_tasks;
    extern symbol measure_id;
    extern symbol members;
    extern symbol members_data;
    extern symbol name;
    extern symbol persistent;
    extern symbol process_measures;
    extern symbol progress;
    extern symbol project;
    extern symbol project_phases;
    extern symbol projects;
    extern symbol projectphase_id;
    extern symbol shortname;
    extern symbol start;
    extern symbol started;
    extern symbol state;
    extern symbol time_load;
    extern symbol tasks;
    extern symbol version;
}

// Member definition
struct Member {
    yesno   active;
    string  fullname;
    symbol  id;
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
    symbol   id;
    
    template <typename F>
    void apply(F& f) const
    {
        f(S::active,  active);
        f(S::fullname,   fullname);
        f(S::id,  id);
    }
};

struct ProcessMeasure {
    symbol   id;
    string   name;
        
    template <typename F>
    void apply(F& f) const
    {
        f(S::id,  id);
        f(S::name,  name);
    }
};

struct ProjectPhase {
    symbol   id;
    string   name;
    
    template <typename F>
    void apply(F& f) const
    {
        f(S::id,  id);
        f(S::name,  name);
    }
};

struct MasterTask {
    symbol id;
    symbol project;
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
    long stamp;
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
    symbol      project;
    symbol      master_task;
    timestamp   created;
    timestamp   started;
    int    progress;
    taskstate   state;
    timestamp   closed;
    timestamp   deadline;
    yesno       hide;
    symbol      measure_id;
    symbol      projectphase_id;
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
    symbol   id;
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

ostream& operator << (ostream& o, const timestamp& s)  { return o << s.stamp; }
ostream& operator << (ostream& o, const taskstate& s)  { 
    return o << ( s == prepared ? "prepared" :
                  s == active   ? "active":
                  s == closed   ? "closed" : "<error>"); 
}
namespace S {
    // symbols
    symbol active = "active";
    symbol closed = "closed";;
    symbol created = "created";;
    symbol deadline = "deadline";;
    symbol description = "description";;
    symbol end = "end";;
    symbol fileversion = "fileversion";
    symbol fullname = "fullname";;
    symbol group = "group";
    symbol hide = "hide";
    symbol id = "id";
    symbol master_task = "master_task";
    symbol master_tasks = "master_tasks";
    symbol measure_id = "measure_id";
    symbol members = "members";
    symbol members_data = "members_data";
    symbol name = "name";
    symbol persistent = "persistent";
    symbol process_measures = "process_measures";
    symbol progress = "progress";
    symbol project = "project";
    symbol project_phases = "project_phases";
    symbol projects = "projects";
    symbol projectphase_id = "projectphase_id";
    symbol shortname = "shortname";
    symbol start = "start";
    symbol started = "started";
    symbol state = "state";
    symbol time_load = "time_load";
    symbol tasks = "tasks";
    symbol version = "version";
}

void initialize_xml_mapping(tinfra::xml::XMLSymbolMapping& xml_mapping)
{
    xml_mapping.map_class_by_traits<TaskMonitor>(symbol("taskmonitor"));
    xml_mapping.map_class_by_traits<ProcessMeasure>(symbol("process-measure"));
    xml_mapping.map_class_by_traits<ProjectPhase>(symbol("project-phase"));
    xml_mapping.map_class_by_traits<Task>(symbol("task"));
    xml_mapping.map_class_by_traits<MasterTask>(symbol("master-task"));
    xml_mapping.map_class_by_traits<TimeLoadEntry>(symbol("time-load-entry"));
    xml_mapping.map_class_by_traits<MemberData>(symbol("member-data"));
    xml_mapping.map_class_by_traits<Project>(symbol("project"));
    xml_mapping.map_class_by_traits<Member>(symbol("member"));
        
    xml_mapping.map_field(S::process_measures,symbol("process-measures"));
    xml_mapping.map_field(S::project_phases,symbol("project-phases"));
    xml_mapping.map_field(S::master_tasks,symbol("master-tasks"));
    xml_mapping.map_field(S::master_task,symbol("master-task"));
    xml_mapping.map_field(S::projectphase_id,symbol("projectphase-id"));
    xml_mapping.map_field(S::measure_id,symbol("measure-id"));
    xml_mapping.map_field(S::members_data,symbol("members-data"));
    xml_mapping.map_field(S::time_load,symbol("time-load"));    
}

void test1()
{
    TaskMonitor tm;
    tm.fileversion.version = 9;
    tm.group.fullname = "Group1";
    
    tinfra::xml::XMLSymbolMapping xml_symbol_mapping;
    initialize_xml_mapping(xml_symbol_mapping);
    
    tm.process_measures.push_back(tinfra::construct<ProcessMeasure>()
                                                           (S::id, symbol("req-1"))
                                                           (S::name, string("XXX")));
    tm.project_phases.push_back(tinfra::construct<ProjectPhase>()
                                                           (S::id, symbol("m1"))
                                                           (S::name, string("Test")));
    tinfra::xml::FileXMLOutputStream xml_out(* cout.rdbuf());
    tinfra::xml::dump(xml_out, symbol("taskmonitor"), tm, xml_symbol_mapping);
}

void test2()
{
    TaskMonitor tm;
    
    tinfra::xml::XMLSymbolMapping xml_symbol_mapping;
    initialize_xml_mapping(xml_symbol_mapping);
    {
        
        tinfra::xml::XMLEventBuffer b;
        tinfra::xml::XMLBufferOutputStream b1(b);
        tinfra::xml::read_xml("taskmonitor2.xml",b1);
        tinfra::xml::XMLBufferInputStream b2(b);
        tinfra::xml::load<TaskMonitor>(b2, symbol("taskmonitor"), tm, xml_symbol_mapping);
        cout << "readed!" << endl;
    }
    {
        tinfra::xml::FileXMLOutputStream xml_out(* cout.rdbuf());
        xml_out.start_document();
        xml_out.set_human_readable(true);
        tinfra::xml::dump(xml_out, symbol("taskmonitor"), tm, xml_symbol_mapping);
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
