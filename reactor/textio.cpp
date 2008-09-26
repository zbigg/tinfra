//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "theg/textio.h"

#include "tinfra/fmt.h"
#include "tinfra/exception.h"
#include <vector>
#include <string>
#include <istream>
#include <ostream>

using tinfra::fmt;

namespace TextIOImpl {

// ContextBasic impl
    
ContextBasic::ContextBasic()        
    : using_context_(true)
{}

static int check_sym(std::string const& input, int pos, tinfra::symbol s)
{
    const std::string& sstr = s.str();
    int sstr_len = sstr.size();
    if( input.compare(pos, sstr_len, sstr, 0, sstr_len ) != 0 ) {
        throw tinfra::generic_exception(fmt("expected %s, got %s") % sstr % input);
    }
    return pos + sstr_len + 1;
}

int ContextBasic::check_context(tinfra::symbol s, std::string const& input)
{
    if( !using_context_ ) return 0;
        
    int pos = 0;
    for( unsigned i = 0; i < context_.size(); ++i ) {        
        pos = check_sym(input, pos, context_[i]);
    }
    pos = check_sym(input, pos, s);
    if( input.at(pos) != '=' )
        throw tinfra::generic_exception(fmt("expected '=' after context, got %s") % input.at(pos));
    return pos + 2;
}
void ContextBasic::realize_context(tinfra::symbol s, std::ostream& output)
{
    if( !using_context_ ) return;
        
    for( unsigned i = 0; i < context_.size(); ++i ) {
        const std::string& sstr = context_[i].str();
        output << sstr << ".";
    }
    output << s.str() << " = ";
}

void ContextBasic::push_context(tinfra::symbol s)
{
    if( !using_context_ ) return;
    context_.push_back(s);
}

void ContextBasic::pop_context()
{
    if( !using_context_ ) return;
        
    context_.erase(context_.end()-1);
}

}

