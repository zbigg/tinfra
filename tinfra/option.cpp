//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "option.h"

#include "tinfra/tstring.h"
#include "tinfra/trace.h"

#include <cstdlib>

namespace tinfra {

//
// option_base
//

const char option_base::NO_SWITCH = 0;
    
option_base::option_base()
{
    option_registry::get().add(this);
}

option_base::option_base(option_list& ol)
{
    ol.add(this);
}

option_base::~option_base()
{
}

//
// option_impl
//

option_impl::option_impl(option_list& ol,char switch_letter, std::string const& name, std::string const& synopsis):
    option_base(ol),
    switch_letter(switch_letter),
    name(name),
    synopsis(synopsis)
{
}

option_impl::option_impl(option_list& ol,std::string const& name, std::string const& synopsis):
    option_base(ol),
    switch_letter(NO_SWITCH),
    name(name),
    synopsis(synopsis)
{
}

option_impl::option_impl(char switch_letter, std::string const& name, std::string const& synopsis):
    switch_letter(switch_letter),
    name(name),
    synopsis(synopsis)
{
}

option_impl::option_impl(std::string const& name, std::string const& synopsis):
    switch_letter(NO_SWITCH),
    name(name),
    synopsis(synopsis)
{
}

char option_impl::get_switch_letter() const
{
    return switch_letter;
}
std::string option_impl::get_name() const
{
    return name;
}

std::string option_impl::get_synopsis() const
{
    return synopsis;
}

//
// option_switch
//

void option_switch::accept(tstring const& input)
{
    if( input.size() == 0 ) {
        this->value_ = !default_value();
    } else if( input == "yes" ||
               input == "y" ||
               input == "1") {
        this->value_ = true;
    } else {
        this->value_ = false;
    }
    accepted_ = true;
}
//
// option_list
//

void option_list::add(option_base* opt)
{
    options.push_back(opt);
}

void remove_params(std::vector<tstring>& from_where, unsigned pos, unsigned how_many = 1)
{
    assert(pos + how_many <= from_where.size());
    
    std::vector<tstring>::iterator remove_begin = from_where.begin()+pos;
    std::vector<tstring>::iterator remove_end   = from_where.begin()+pos + how_many;
    from_where.erase(remove_begin, remove_end); 
}

void option_list::parse(std::vector<tstring>& params)
{
    unsigned i = 0;
    while( i < params.size()) {
        tstring current = params[i];
        if( current.size() < 2 ) {
            i+=1;
            continue;
        }

        tstring option_name;
        tstring option_value;
        bool argument_present;
        option_base* opt;
        if( std::strncmp(current.data(), "--", 2) == 0 )  {
            if( current.size() == 2 ) {
                remove_params(params,i,1);
                // -- is an end of parameters
                break;
            }
            
            // parse only long style, gnu-like options (--foo-bar, --foo-bar=AAA, --foo-bar AAA)
            current = current.substr(2);
            
            size_t eq_pos = current.find_first_of('=');
            argument_present = (eq_pos != tstring::npos);
            if( argument_present ) {
                // and option=argument pair
                option_name  = current.substr(0,eq_pos);
                option_value = current.substr(eq_pos+1);
            } else {
                // only option, no argument
                option_name = current;
            }
            opt = find_by_name(option_name);
        } else if( current[0] == '-' ) {
            // parse short options:
            // -I
            // -Iparam
            // -I param
            const char shortcut = current[1];
            
            option_value = current.substr(2);
            argument_present = (option_value.size() != 0);
            opt = find_by_shortcut(shortcut);
	} else {
            i+=1;
            continue;
        }
        
        int arguments_eaten = 1;
        
        if( opt == 0 ) {
            // ignore unknown option
            // TODO, invent other strategy for unknown options
            i+=1;
            continue;
        }
        
        if( opt->needs_argument() ) {
            if( !argument_present ) {
                if( i == params.size() - 1 ) {
                    // we need argument but are at last element, so 
                    // throw!
                    assert(false);
                    throw;
                }
                option_value = params[i+1];
                arguments_eaten += 1;
            }
            
            opt->accept(option_value);
        } else {
            if( argument_present ) 
                opt->accept(option_value);
            else
                opt->accept("");
        }
        remove_params(params, i, arguments_eaten);
    }
}

void option_list::print_help(tinfra::output_stream& out)
{
    std::ostringstream tmp;
    for(option_list_t::const_iterator i = options.begin(); i != options.end(); ++i) 
    {
        const option_base* opt = *i;
        tmp << "    ";
        
        const char switch_letter =  opt->get_switch_letter();
        if( switch_letter != option_base::NO_SWITCH )
            tmp << "-" << switch_letter << ", ";
        
        
        tmp  << "--" << opt->get_name();
        if( opt->needs_argument() ) {
            tmp  << " ARG";
        }
        
        tmp << "\t" << opt->get_synopsis() << "\n";
    }
    out.write(tmp.str());
}
option_base* option_list::find_by_name(tstring const& name)
{
    for(option_list_t::const_iterator i = options.begin(); i != options.end(); ++i) 
    {
        option_base* opt = *i;
        
        if( name == opt->get_name() ) 
            return opt;
        
        // TODO: add - and _ insensitive search
    }
    return 0;
}

option_base* option_list::find_by_shortcut(char shortcut)
{
    for(option_list_t::const_iterator i = options.begin(); i != options.end(); ++i) 
    {
        option_base* opt = *i;
        
        if( shortcut == opt->get_switch_letter() ) 
            return opt;
    }
    return 0;
}

option_list::option_list_t option_list::get_options()
{
    return options;
}

//
// option_registry
//
static option_list* default_option_list = 0;
static void delete_default_option_list()
{
    delete default_option_list;
}

option_list& option_registry::get()
{
    if( default_option_list == 0 ) {
        default_option_list = new option_list();
        std::atexit(delete_default_option_list);
    }
    return *default_option_list;
}

} // end namespace tinfra

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

