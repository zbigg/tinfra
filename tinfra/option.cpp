#include "option.h"

#include "tinfra/tstring.h"

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
// option_list
//

void option_list::add(option_base* opt)
{
    options.push_back(opt);
}

void remove_params(std::vector<tstring>& from_where, int pos, int how_many = 1)
{
    assert(pos + how_many < from_where.size());
    
    std::vector<tstring>::iterator remove_begin = from_where.begin()+pos;
    std::vector<tstring>::iterator remove_end   = from_where.begin()+pos + how_many;
    from_where.erase(remove_begin, remove_end); 
}

void option_list::parse(std::vector<tstring>& params)
{
    int i = 0;
    while( i < params.size()) {
        tstring current = params[i];
        if( current.size() < 2 ) {
            i+=1;
            continue;
        }
        
        if( std::strncmp(current.data(), "--", 2) != 0 )  {
            i+=1;
            continue;
        }
        
        if( current.size() == 2 ) {
            remove_params(params,i,1);
            // -- is an end of parameters
            break;
        }
        
        // currently we support only long style, gnu-like options (--foo-bar)
        current = current.substr(2);
        tstring option_name;
        tstring option_value;
        
        size_t eq_pos = current.find_first_of('=');
        const bool argument_present = (eq_pos != tstring::npos);
        if( argument_present ) {
            // and option=argument pair
            option_name  = current.substr(0,eq_pos);
            option_value = current.substr(eq_pos+1);
        } else {
            // only option, no argument
            option_name = current;
        }
        int arguments_eaten = 1;
        option_base* opt = find_option(option_name);
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
            opt->accept("");
        }
        remove_params(params, i, arguments_eaten);
    }
}

option_base* option_list::find_option(tstring const& name)
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
