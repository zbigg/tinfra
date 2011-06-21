//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/trace.h"
#include "tinfra/fmt.h"
#include "tinfra/cmd.h"
#include "tinfra/tstring.h"
#include "tinfra/stream.h"

#include <sstream>
#include <list>
#include <iterator>
#include <algorithm>
#include <stdexcept>

namespace tinfra { 
namespace trace {
    auto_register_tracer __tinfra_global_tracer("global");
    tinfra::trace::auto_register_tracer error_tracer("errors", true);
}}

tinfra::trace::tracer& __tinfra_tracer_adaptable = tinfra::trace::__tinfra_global_tracer;

namespace tinfra { 
namespace trace {

//
// tracer implementation
//
void tracer::trace(location const& loc, const char* message)
{
    if( !is_enabled() ) return;
        
    std::ostringstream out;
    if( name_ && std::strlen(name_) ) {
        out << '[' << name_ << "] ";
    }
    out << loc.filename << ':' << loc.line;
    if( loc.name ) {
        out << '[' << loc.name << ']';
    } 
    out << ": " << message << std::endl;
    tinfra::err.write( out.str() );
}

//
// tracer registry
//

auto_register_tracer::auto_register_tracer(const char* name, bool enabled):
        tracer(name, enabled)
{
    static_registry<tracer>::register_element(this);
}

auto_register_tracer::~auto_register_tracer()
{
    static_registry<tracer>::register_element(this);
}

std::vector<tracer*> get_global_tracers()
{
    return static_registry<tracer>::elements();
}


void print_tracer_usage(tstring const&)
{
    std::ostringstream tmp;
    tmp << "Trace system arguments:\n"
           "    --tracer-help        print tracers list and usage\n"
           "    --tracer-enable=name enable specific tracer\n"
           "\n"
           "Available tracers:\n";;
    std::vector<tracer*> const r = get_global_tracers();
    for(std::vector<tracer*>::const_iterator i = r.begin(); i != r.end(); ++i ) {
        tracer* t = *i;
        tmp << "    " << t->name() << std::endl;
    }
    tinfra::err.write(tmp.str());
}

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

bool enable_tracer_by_mask(tstring const& mask)
{
    std::vector<tracer*> const r = get_global_tracers();
    bool anything = false;
    for(std::vector<tracer*>::const_iterator i = r.begin(); i != r.end(); ++i )
    {
        tracer* t = *i;
        if( !matches(mask,t->name()) ) 
            continue;
        tinfra::cmd::inform(fmt("trace: enabling tracer '%s'") % t->name());
        t->enable(true);
        anything = true;
    }
    if( !anything ) {
        TINFRA_LOG_ERROR(fmt("no tracer matches mask '%s'") % mask);
    }
    return anything;
}

void enable_tracer_by_mask_cmd(tstring const& mask)
{
    if( mask.size() == 0 ) {
        print_tracer_usage();
        throw std::logic_error(fmt("invalid tracer name: '%s'") % mask);
    }
    enable_tracer_by_mask(mask);
}

static const tinfra::tstring HELP_OPTION = "--tracer-help";
static const tinfra::tstring ENABLE_OPTION = "--tracer-enable";

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
void process_params(int& argc, char** argv)
{
    using std::strcmp;
    using std::strncmp;
    const char* env_mask = ::getenv("TINFRA_TRACE");
    if( env_mask != 0 ) {
    	    enable_tracer_by_mask(env_mask);
    }
    for( int i = 1; i < argc; ) {
        if( HELP_OPTION == argv[i] ) {
            print_tracer_usage();
            ::exit(0);
        }
        if( ENABLE_OPTION == argv[i]) {
            if( i == argc-1 ) {
                print_tracer_usage();
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
}} // end namespace tinfra::trace
