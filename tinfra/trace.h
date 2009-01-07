//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_trace_h__
#define __tinfra_trace_h__

#include <vector>
#include "tinfra/tstring.h"

#if defined _MSC_VER
//
// excerpt from MSDN:
//
//  __FUNCDNAME__
//      ... returns the decorated name of the enclosing function (as a string)...
// __FUNCSIG__
//      ... returns the signature of the enclosing function (as a string). ...
// __FUNCTION__
//      ... returns the undecorated name of the enclosing function (as a string). ...
//
#define TINFRA_PRETTY_FUNCTION __FUNCSIG__
#elif defined(__GNUC__)
#define TINFRA_PRETTY_FUNCTION __PRETTY_FUNCTION__
#else
#define TINFRA_PRETTY_FUNCTION __func__
#endif
    
namespace tinfra {
namespace trace {
    
struct location {
    const char* filename;
    int         line;
    const char* name;
};

class tracer {
public:
    tracer(const char* name, bool enabled = false)
        : name_(name), enabled_(enabled)

    {
    }
    
    tracer(const char* name, tracer const& other)
        : name_(name), enabled_(other.is_enabled())

    {
    }
        
    void trace(const char* file_name, int line, const char* where, const char* message)
    {
        if( !is_enabled() ) return;
        location loc = { file_name, line, where };
        trace(loc, message);
    }
    
    void trace(location const& l, const char* message);
    
    bool is_enabled() const    { return enabled_; }
    void enable(bool e = true) { enabled_ = e; }
    const char* name() const   { return name_; }
private:
    const char* name_;
    bool        enabled_;
};

class auto_register_tracer: public tracer {
public:
    auto_register_tracer(const char* name);
    ~auto_register_tracer();
};

class exit_tracer {
public:
    exit_tracer(const char* filename, int line, const char* name, tracer& other):
        start_line(line),
        end_line(line),
        filename(filename),
        name(name),
        t(other)
    {
        t.trace(filename, start_line, name, "entering");
    }
    ~exit_tracer()
    
    {
        t.trace(filename, end_line, name, "exiting");
    }
    
    void set_end_line(int el) { end_line = el; }
private:
    int         start_line;
    int         end_line;
    const char* filename;
    const char* name;
    tracer&     t;
};

std::vector<tracer*> get_global_tracers();

void process_params(int& argc, char** argv);
void print_tracer_usage(tstring const& msg = "");

bool enable_tracer_by_name(tstring const& name);

} } // end of namespace tinfra::trace

#include <sstream>

extern tinfra::trace::tracer&              __tinfra_tracer_adaptable;
extern tinfra::trace::auto_register_tracer __tinfra_global_tracer;


#define TINFRA_TRACER __tinfra_tracer_adaptable
#define TINFRA_TRACE_MSG(msg) do { TINFRA_TRACER.trace(__FILE__, __LINE__, __func__, msg); } while(false)
#define TINFRA_TRACE_VAR(name) do {    \
  std::ostringstream s;                  \
  s << #name << " = '" << (name) << "'";  \
  TINFRA_TRACE_MSG(s.str().c_str());  } while(false)

#define TINFRA_USE_TRACER(name) tinfra::trace::tracer& __tinfra_tracer_adaptable(name)


#define TINFRA_CALL_TRACE() \
tinfra::trace::entry_exit_tracer __tinfra_tracer_entry_exit(TINFRA_PRETTY_FUNCTION, __tinfra_tracer_adaptable)


#define TINFRA_EXIT_TRACE() __tinfra_tracer_entry_exit.set_end_line(__LINE__)

#define TINFRA_MODULE_TRACER(name) namespace { TINFRA_PUBLIC_TRACER(name); }

#define TINFRA_PUBLIC_TRACER(name)                   \
tinfra::trace::auto_register_tracer     name(#name)

#endif
