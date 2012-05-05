//
// Copyright (c) 2009-2011, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/platform.h" 

#include "trace.h" // we implement this

#include "tinfra/logger.h" // for tinfra::logger
#include "tinfra/fmt.h"
#include "tinfra/cmd.h"
#include "tinfra/tstring.h"
#include "tinfra/stream.h"

#include <sstream>
#include <list>
#include <iterator>
#include <algorithm>
#include <stdexcept>
#include <cstdlib>

namespace tinfra { 

//
// static instances
//
    
module_tracer global_tracer("global", 2);
module_tracer tinfra_tracer(global_tracer, "tinfra", 2);

//
// tracer implementation
//

void tracer::set_enabled(bool new_value)
{
    this->enabled = new_value;
}

bool tracer::is_enabled_inherit()
{
    tracer* cur_tracer=this;
    bool result;
    do {
        result = cur_tracer->is_enabled();
        if( result )
            break;
        tracer* next = cur_tracer->get_parent();
        if( next == cur_tracer )
            break;
        cur_tracer = next;
    } while( cur_tracer );
    this->enabled = result;
    return result;
}

void tracer::trace(tstring const& message, source_location const& sloc)
{
    // TBD, inheritance mechanism is fucked up!
    // if( !is_enabled_inherit() )
    if( !is_enabled() ) 
        return;
        
    const tstring name(this->get_name());
    logger log(name);
    log.log(tinfra::TRACE, message, sloc);
}

tracer::tracer(tracer* parent, const char* name, bool enabled):
    enabled(enabled),
    name(name),
    parent(parent)
{
}

tracer::tracer(const char* name, bool enabled):
    enabled(enabled),
    name(name),
    parent(0)
{
}

tracer::tracer(tracer& parent, const char* name, bool enabled):
    enabled(enabled),
    name(name),
    parent(&parent)
{
}
//
// call_tracer
//

call_tracer::call_tracer(tinfra::source_location const& sloc):
    tracer(&global_tracer, sloc.name, global_tracer.is_enabled())
{
    trace("entering", sloc);
}

call_tracer::call_tracer(tracer& parent, tinfra::source_location const& sloc):
    tracer(&parent, sloc.name, parent.is_enabled())
{
    trace("entering", sloc);
}

call_tracer::call_tracer(tracer& parent, const char* name):
    tracer(&parent, name, parent.is_enabled())
{
    trace("entering", TINFRA_NULL_SOURCE_LOCATION());
}

call_tracer::~call_tracer()
{
    trace("exiting", TINFRA_NULL_SOURCE_LOCATION());
}

//
// module_tracer
//

module_tracer::module_tracer(tracer& parent, const char* name, int verbosity_level):
    public_tracer(parent, name, verbosity_level)
{
}
module_tracer::module_tracer(const char* name, int verbosity_level):
    public_tracer(name, verbosity_level)
{
}

//
// public_tracer
//
//  int         verbosity_level;  
//  bool        inherit_enabled;

public_tracer::public_tracer(tracer& parent, const char* name, int verbosity_level):
    tracer(&parent, name, false),
    verbosity_level(verbosity_level),
    inherit_enabled(true)
{
    static_registry<public_tracer>::register_element(this);
}

public_tracer::public_tracer(const char* name, int verbosity_level):
    tracer(&global_tracer, name, false),
    verbosity_level(verbosity_level),
    inherit_enabled(true)
{
    static_registry<public_tracer>::register_element(this);
}

public_tracer::~public_tracer()
{
    static_registry<public_tracer>::unregister_element(this);
}

int  public_tracer::get_verbosity_level()
{
    return this->verbosity_level;
}
    
void public_tracer::set_inherit_enabled(bool inherit_enabled)
{
    this->inherit_enabled = inherit_enabled;
    if( this->inherit_enabled )
        this->is_enabled_inherit();
}

//
// public_tracer statics -> tracer registry
//

static bool matches(tstring const& mask, tstring const& str);
static void enable_tracer_by_mask_cmd(tstring const& mask);
static void remove_arg(int i, int& argc, char** argv);

//static 
void public_tracer::enable_by_mask(tstring const& mask)
{
    std::vector<public_tracer*> const r = get_global_tracers();
    bool anything = false;
    for(std::vector<public_tracer*>::const_iterator i = r.begin(); i != r.end(); ++i )
    {
        tracer* t = *i;
        if( !matches(mask,t->get_name()) ) 
            continue;
        tinfra::cmd::inform(fmt("trace: enabling tracer '%s'") % t->get_name());
        t->set_enabled(true);
        anything = true;
    }
    if( !anything ) {
        log_error(fmt("no tracer matches mask '%s'") % mask);
    }
    //return anything;
}

//static 
void public_tracer::enable_by_level(int level)
{
    std::vector<public_tracer*> const r = get_global_tracers();
    bool anything = false;
    for(std::vector<public_tracer*>::const_iterator i = r.begin(); i != r.end(); ++i )
    {
        tracer* t = *i;
    }
}

//static 
void public_tracer::enable_all(bool enable)
{
    std::vector<public_tracer*> const r = get_global_tracers();
    bool anything = false;
    for(std::vector<public_tracer*>::const_iterator i = r.begin(); i != r.end(); ++i )
    {
        tracer* t = *i;
        t->set_enabled(enable);
    }
}

// interrogate
//static 
std::vector<public_tracer*> public_tracer::get_global_tracers()
{
    return static_registry<public_tracer>::elements();
}

/// process trace params
///
/// consumes (removes!) trace-related params from list

static const tinfra::tstring HELP_OPTION = "--tracer-help";
static const tinfra::tstring ENABLE_OPTION = "--tracer-enable";

//static 
void public_tracer::process_params(int& argc, char** argv)
{
    using std::strcmp;
    using std::strncmp;
    const char* env_mask = std::getenv("TINFRA_TRACE");
    if( env_mask != 0 ) {
    	    enable_by_mask(env_mask);
    }
    for( int i = 1; i < argc; ) {
        if( HELP_OPTION == argv[i] ) {
            print_tracer_usage(tinfra::err);
            ::exit(0);
        }
        if( ENABLE_OPTION == argv[i]) {
            if( i == argc-1 ) {
                print_tracer_usage(tinfra::err);
                throw std::logic_error("--tracer-enable: missing tracer name");
            }
            enable_tracer_by_mask_cmd(argv[i+1]);
            remove_arg(i, argc, argv);
            remove_arg(i, argc, argv);
            continue;
        }
        if(    strncmp(argv[i], ENABLE_OPTION.data(), ENABLE_OPTION.size()) == 0
            && argv[i][ENABLE_OPTION.size()] == '=') 
        {
            const char* name = argv[i] + ENABLE_OPTION.size()+1;
            enable_tracer_by_mask_cmd(name);
            
            remove_arg(i, argc, argv);
            continue;
        }
        ++i;
    }
}

//static 
void public_tracer::print_tracer_usage(tinfra::output_stream& out, tstring const&)
{
    std::ostringstream tmp;
    tmp << "Trace system arguments:\n"
           "    --tracer-help        print tracers list and usage\n"
           "    --tracer-enable=name enable specific tracer\n"
           "\n"
           "Available tracers:\n";;
    std::vector<public_tracer*> const r = get_global_tracers();
    for(std::vector<public_tracer*>::const_iterator i = r.begin(); i != r.end(); ++i ) {
        tracer* t = *i;
        const char* name = t->get_name();
        if( !name ) // protect against misconstructed tracers 
            continue;
        tmp << "    " << t->get_name() << std::endl;
    }
    out.write(tmp.str());
}

//
// implementation details
//

static bool matches(tstring const& mask, tstring const& str)
{
    if( mask == str )
        return true;
    if( mask.size() > 0 && mask[mask.size()-1] == '*') {
        size_t const_part_len = mask.size()-1;
        
        if( str.size() >= const_part_len &&
            str.substr(0,const_part_len) == mask.substr(0, const_part_len) )
            return true;
    }
    return false;
}

static void enable_tracer_by_mask_cmd(tstring const& mask)
{
    if( mask.size() == 0 ) {
        public_tracer::print_tracer_usage(tinfra::err);
		const std::string error_message = (fmt("invalid tracer name: '%s'") % mask).str();
        throw std::logic_error(error_message);
    }
    public_tracer::enable_by_mask(mask);
}


static void remove_arg(int i, int& argc, char** argv)
{   
    char** dest = argv + i;
    char** src   = argv + i + 1;
    const int size = argc - i - 1;
    
    if( size > 0  )
    memmove(dest, src, size*sizeof(char*));
    argv[argc-1] = 0;
    argc--;
}

} // end namespace tinfra

