//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_trace_h_included
#define tinfra_trace_h_included

#include <vector>
#include "tinfra/platform.h"
#include "tinfra/tstring.h"
#include "static_registry.h"
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
#define TINFRA_SHORT_FUNCTION __FUNCTION__
#elif defined(__GNUC__)
#define TINFRA_PRETTY_FUNCTION __PRETTY_FUNCTION__
#define TINFRA_SHORT_FUNCTION __func__
#else
#define TINFRA_PRETTY_FUNCTION __func__
#define TINFRA_SHORT_FUNCTION __func__
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

class auto_register_tracer: public tracer                       
{
public:
    auto_register_tracer(const char* name, bool enabled = false);
    ~auto_register_tracer();
};

class exit_tracer {
public:
    exit_tracer(const char* filename, int line, const char* name, tracer& other):
        end_line(line),
        filename(filename),
        name(name),
        t(other)
    {
        t.trace(filename, line, name, "entering");
    }
    ~exit_tracer()
    
    {
        t.trace(filename, end_line, name, "exiting");
    }
    
    void set_end_line(int el) { end_line = el; }
private:
    int         end_line;
    const char* filename;
    const char* name;
    tracer&     t;
};

std::vector<tracer*> get_global_tracers();

void process_params(int& argc, char** argv);
void print_tracer_usage(tstring const& msg = "");

bool enable_tracer_by_mask(tstring const& mask);

extern tinfra::trace::auto_register_tracer error_tracer;


} } // end of namespace tinfra::trace

#include <sstream>

extern tinfra::trace::tracer&              __tinfra_tracer_adaptable;
extern tinfra::trace::auto_register_tracer __tinfra_global_tracer;

#define TINFRA_LOG_ERROR(msg) tinfra::trace::error_tracer.trace(__FILE__, __LINE__, TINFRA_SHORT_FUNCTION, (msg))

#define TINFRA_TRACER __tinfra_tracer_adaptable


#define TINFRA_TRACE_MSG(msg) do { if(TINFRA_UNLIKELY(TINFRA_TRACER.is_enabled())) { \
  TINFRA_TRACER.trace(__FILE__, __LINE__, TINFRA_SHORT_FUNCTION, (msg)); }} while(false)

#define TINFRA_TRACE_VAR(name) do { if( TINFRA_UNLIKELY(TINFRA_TRACER.is_enabled())) {    \
  std::ostringstream _ojejuku_kejku_akuku;                  \
  _ojejuku_kejku_akuku << #name << " = '" << (name) << "'";  \
  TINFRA_TRACE_MSG(_ojejuku_kejku_akuku.str().c_str()); }} while(false)

#define TINFRA_USE_TRACER(name) tinfra::trace::tracer& __tinfra_tracer_adaptable(name)


#define TINFRA_CALL_TRACE() \
tinfra::trace::exit_tracer __tinfra_tracer_entry_exit(__FILE__, __LINE__, TINFRA_SHORT_FUNCTION, __tinfra_tracer_adaptable)


#define TINFRA_EXIT_TRACE() __tinfra_tracer_entry_exit.set_end_line(__LINE__)

#define TINFRA_MODULE_TRACER(name) namespace { \
    TINFRA_PUBLIC_TRACER(name); \
    tinfra::trace::tracer& __tinfra_tracer_adaptable(name); \
    } \
    int TINFRA_MODULE_TRACER_ ## name ## _statement_enforcement

#define TINFRA_PUBLIC_TRACER(name)                   \
tinfra::trace::auto_register_tracer     name(#name)

#endif
