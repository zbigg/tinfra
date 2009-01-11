//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "tinfra/trace.h"
#include "tinfra/fmt.h"

#include <sstream>
#include <list>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace tinfra { 
namespace trace {
    auto_register_tracer __tinfra_global_tracer("global");
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
    std::cerr << out.str();
}

//
// tracer registry
//

typedef std::list<tracer*> tracer_registry;

static tracer_registry& global_tracer_registry()
{
    static tracer_registry iii;
    return iii;
}

std::vector<tracer*> get_global_tracers()
{
    std::vector<tracer*> result;
    
    {
        // TODO with lock
        tracer_registry& r = global_tracer_registry();
        std::copy(r.begin(), r.end(), 
            std::back_insert_iterator< std::vector<tracer*> >(result));
    }
    return result;
}

//
// auto_register_tracer
//
auto_register_tracer::auto_register_tracer(const char* name)
    : tracer(name) 
{
    // TODO with lock
    global_tracer_registry().push_back(this);
}

auto_register_tracer::~auto_register_tracer() 
{
    // TODO with lock
    global_tracer_registry().remove(this);
}

void print_tracer_usage(tstring const& msg)
{
    std::cerr << "Trace system arguments:\n"
                 "    --tracer-help        print tracers list and usage\n"
                 "    --tracer-enable=name enable specific tracer\n"
                 "\n"
                 "Available tracers:\n";;
    tracer_registry& r = global_tracer_registry();
    for(tracer_registry::const_iterator i = r.begin(); i != r.end(); ++i ) {
        tracer* t = *i;
        std::cerr << "    " << t->name() << std::endl;
    }
}

bool enable_tracer_by_name(tstring const& name)
{
    tracer_registry& r = global_tracer_registry();
    for(tracer_registry::const_iterator i = r.begin(); i != r.end(); ++i )
    {
        tracer* t = *i;
        if( name != t->name() ) continue;
            
        t->enable(true);
        return true;
    }
    
    return false;
}

void enable_tracer_by_name_cmd(tstring const& name)
{
    if( name.size() == 0 || !enable_tracer_by_name(name) ) {
        print_tracer_usage();
        // THROW_ANALYSIS: this is "bad command line parameter" error
        throw std::logic_error(fmt("unknown or invalid tracer name: '%s'") % name);
    }
}

static const tstring HELP_COMMAND = "--tracer-help";
static const tstring ENABLE_COMMAND = "--tracer-enable";

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
    
    for( int i = 1; i < argc; ++i ) {
        if( HELP_COMMAND == argv[i] ) {
            print_tracer_usage();
            ::exit(0);
        }
        if( ENABLE_COMMAND == argv[i]) {
            if( i == argc-1 ) {
                print_tracer_usage();
                // THROW_ANALYSIS: this is "bad command line parameter" error
                throw std::logic_error("--tracer-enable: missing tracer name");
            }
            enable_tracer_by_name_cmd(argv[i+1]);
            remove_arg(i, argc, argv);
            remove_arg(i, argc, argv);
            --i;
            continue;
        }
        if(    strncmp(argv[i], ENABLE_COMMAND.data(), ENABLE_COMMAND.size()) == 0
            && argv[i][ENABLE_COMMAND.size()] == '=') 
        {
            const char* name = argv[i] + ENABLE_COMMAND.size()+1;
            enable_tracer_by_name_cmd(name);
            
            remove_arg(i, argc, argv);
            continue;
        }
    }
}
}} // end namespace tinfra::trace
