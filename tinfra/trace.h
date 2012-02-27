//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_trace_h_included
#define tinfra_trace_h_included

#include <vector>
#include "tinfra/platform.h" // for TINFRA_UNLIKELY, TINFRA_SHORT_FUNCTION
#include "tinfra/tstring.h"
#include "tinfra/stream.h"   // for tinfra::output_stream

#include "tinfra/static_registry.h"

#include <sstream>

namespace tinfra {
    
/**
 tinfra trace subsystem
 
    This subsystem allows easy, robust and effective trace of runtime data in program.
    <blablabla>
    
    Use case:
    
    module called foo
    
    foo.cpp:
    namespace foo {
    tinfra::module_tracer foo_trace("foo");
    void a(std::string const& a) {
        tinfra::call_tracer fun_trace(foo_trace, "a");
        
        
        TINFRA_TRACE(fun_trace, "a=" << a);
        TINFRA_GLOBAL_TRACE("a=" << a); // equivalent to
        TINFRA_TRACE(tinfra::global_trace,  "a=" << a); // equivalent to
        
        
    }
    
    Fail tracer is trace utility that enable automagic trace of values
    only if failure occurs.
    The key macro set TINFRA_TRACE_IF_FAIL** mean: dump value of these
    variables only in case of failure.
    fail tracer:
    const std::string a;
    int b;
    {
        tinfra::fail_tracer fail_tracer(foo_trace); // use custom parent tracer
        tinfra::fail_tracer fail_tracer(); // use global tracer as delegate     
        TINFRA_TRACE_IF_FAIL_NOCOPY(fail_tracer, a)
        TINFRA_TRACE_IF_FAIL(fail_tracer, b)
        ...
        // inferior code
        
        // implicit destructor of fail_tracer 
    }
    This will add 'a' reference to temporary list of values "potentially"
    interesting in case of failure.
    'a' is passed as NOCOPY, so it will be added as reference so a MUST BE
    constant (at best, also declared as const).
    'b' is passed as copy, so it may be safely modified by inferr
    
    Initialization architecture.
    ----------------------------
    
    The module_tracer instances can be used only and only after static-
    initialization phase.
    
    Global tracer can be used always, but it might be not yet in state
    resembling program arguments and environment. So, it is disabled by
    default.
    
    module_tracer adds self into list of globally visible tracers. Each
    of such tracer may be enabled in tinfra::tracer::process_args(...) by:
      * TINFRA_TRACE environment variable
      * --tracer-enable=... arguments
    Additionally, application may execute tracer::enable explicitly at
    any point of lifetime.
    
    Changing global trace level.
    Each module_tracer has some parent (by default - global tracer).
    The global tracer enabled-level can be set with just 
      tinfra::tracer_list::enable_by_mask(tstring const& mask)
      tinfra::tracer_list::enable_by_level(int level)
      tinfra::tracer_list::enable_all(bool enable)
      
    It will be propagated to all named tracers (module and global tracers).
    
    It is assumed that module_tracers are always allocated in static storage.
    It is assumed that module tracer instance are always safe to be accessed
    even ouside of object lifetime (before consturction/after destruction).
    
    function call tracer are volatile, they can appear and disappear for moments
    and thus are not registered.
    
    If function call tracer shall be publicly accessible, then an instance of
    function_public_tracer should be made and used as a parent for actual 
    call_tracer. Example:
    
    static tinfra::function_tracer capture_command_tracer("tinfra::capture_command", tinfra::tinfra_tracer);
    std::string capture_command(...) {
        tinfra::call_tracer cc_tracer(capture_command_tracer, "capture_command", TINFRA_SOURCE_LOCATION);
        
        TINFRA_TRACE(cc_tracer, "blablablbla"); 
        // but if call trace is not needed then it is enough to
        TINFRA_TRACE(capture_command_tracer, "blablablbla"); 
    }
    
    
  */

#define TINFRA_TRACE(tracer,cout_like_expr)  TINFRA_TRACE_IMPL(tracer,                  cout_like_expr)
#define TINFRA_GLOBAL_TRACE(cout_like_expr)  TINFRA_TRACE_IMPL(::tinfra::global_tracer, cout_like_expr)

#define TINFRA_TRACE_VAR(tracer,variable)  TINFRA_TRACE(tracer, #variable " = '" << (variable) << "'" )
#define TINFRA_GLOBAL_TRACE_VAR(variable)  TINFRA_TRACE_VAR(::tinfra::global_tracer, variable)


/// Return current source code location.
///
/// Evaluates to const trace::location that represents current line in source code.
///
/// Usage example:
///    tinfra::log_error("something unexpeced", TINFRA_SOURCE_LOCATION());
/// Complexity constant, nomemory.
#define TINFRA_SOURCE_LOCATION() tinfra::make_source_location(__FILE__, __LINE__, TINFRA_SHORT_FUNCTION)

/// Logs enter and exit.
/// 
/// All traces directed by this tracer are marked with function name (?)

/// Locates source code location. 
struct source_location {
    /// Source file name.
    ///
    /// It is undefined if filename is absolute and/or 
    /// unique. It is compilation environment dependent.
    const char* filename;
    
    /// Source line number.
    int         line;
    
    /// Name of entity doing a trace.
    /// Usually a function or method name.
    const char* name;
};

class tracer {
    bool        enabled;
    const char* name;
    tracer*     parent;
    
public:
    
    void        set_enabled(bool new_value = true);
    bool        is_enabled() const; // inline
    bool        is_enabled_inherit();
    const char* get_name() const;
    tracer*     get_parent() const; // inline
    
    void trace(tstring const& message, tinfra::source_location const& sloc);
protected:
    tracer(tracer* parent, const char* name, bool enabled);
};

/// Function entry/exit tracer.
///
/// Logs function all on entry and on exit.
/// Inherits current enabled level from parent tracer.
class call_tracer: public tracer {
public:
    call_tracer(tinfra::source_location const& sloc);
    call_tracer(tracer& parent, tinfra::source_location const& sloc);
    call_tracer(tracer& parent, const char* name);
    ~call_tracer();
    
    // trace is inherited from tracer (?)
};

/// Named, publicly registered tracer.
///
/// These tracers are registered on linked-list
/// in contrusctors.
/// They shall be instantiated in static storage.
/// Their enabled() status will be dynamically
/// modified using tinfra::tracer_list::enable*
/// methods.
class public_tracer: public tracer {
    int         verbosity_level;
    
    bool        inherit_enabled;
public:
    public_tracer(tracer& parent, const char* name, int verbosity_level = 2);
    public_tracer(const char* name, int verbosity_level = 2);
    ~public_tracer();

    int         get_verbosity_level();
    
    void        set_inherit_enabled(bool inherit_enabled = true);
    
    
    // static enablers for globally registered tracers
    static void enable_by_mask(tstring const& mask);
    static void enable_by_level(int level);
    static void enable_all(bool enable);
    
    // interrogate
    static std::vector<public_tracer*> get_global_tracers();
    
    /// process trace params
    ///
    /// consumes (removes!) trace-related params from list
    static void process_params(int& argc, char** argv);
    
    static void print_tracer_usage(tinfra::output_stream& out, tstring const& msg = "");
};

/// Module named public tracer
/// 
/// A tracer for module/class.
/// It is public tracer, so should be module/class local
/// object in static storage.
class module_tracer: public public_tracer {    
public:
    module_tracer(tracer& parent, const char* name, int verbosity_level = 2);
    module_tracer(const char* name, int verbosity_level = 2);
};

/// Function named public tracer
/// 
/// A tracer for function/method.
/// It is public tracer, it should be declared in
/// in static storage.
class function_tracer: public public_tracer {    
public:
    function_tracer(tracer& parent, const char* name, int verbosity_level = 2);
    function_tracer(const char* name, int verbosity_level = 2);
};


extern module_tracer global_tracer;
extern module_tracer tinfra_tracer; 

//
// implementation detail
//

tinfra::source_location make_source_location(const char* filename, int line, const char* func);

#define TINFRA_TRACE_IMPL(tracer, cout_like_expr) do {   \
        if(TINFRA_UNLIKELY(tracer.is_enabled())) {       \
            std::ostringstream _tinfra_trace_the_buffer; \
            _tinfra_trace_the_buffer << cout_like_expr;       \
            tracer.trace(_tinfra_trace_the_buffer.str(), TINFRA_SOURCE_LOCATION()); \
        } } while(false)

#define TINFRA_NULL_SOURCE_LOCATION() \
    tinfra::make_source_location(0,0,0)
            
// backwards compatibility

/*
// shouldn't it be moved to logger.h ?
#define TINFRA_LOG_ERROR(msg) tinfra::global_tracer.trace((msg), TINFRA_SOURCE_LOCATION())

#define TINFRA_TRACE_MSG(msg) TINFRA_GLOBAL_TRACE( (msg) )
#define TINFRA_TRACE_STRM(msg) TINFRA_GLOBAL_TRACE( (msg) )

#define TINFRA_TRACE_VAR(name) TINFRA_GLOBAL_TRACE( #name " = \"" << name << "\"" )


#define TINFRA_CALL_TRACE() \
tinfra::call_tracer _tinfra_call_tracer(TINFRA_SOURCE_LOCATION())

#define TINFRA_EXIT_TRACE() 
*/

//
// implementation
//

//
// tracer inlines
//

inline bool tracer::is_enabled() const
{
    return this->enabled;
}

inline tracer* tracer::get_parent() const
{
    return this->parent;
}

inline const char* tracer::get_name() const
{
    return this->name;
}
//
// helpers
//
inline tinfra::source_location  make_source_location(const char* filename, int line, const char* func)
{
    source_location loc = { filename, line, func };
    return loc;
}

} // end namespace tinfra

#endif
