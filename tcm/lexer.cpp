#include "lexer.h" // we implement this: lexer_info, string_lexer

#include <tinfra/trace.h>
#include <tinfra/tstring.h>

#include <cassert>

static const int count_groups(std::string const& regexp)
{
    int result = 0;
    bool in_quote = false;
    for(int i = 0; i < regexp.size(); ++i ) {
        const char c = regexp[i];
        if( c == '\\' ) {
            ++i;
            continue;
        }
        if( in_quote && c != '"' ) 
            continue;
        
        if( c == '"' ) {
            in_quote=true;
        } else if ( c == '(' )
            result++;
    }
    return result;
}

//
// lexer_info implementation
//

//STATIC_AUTO_TEST(count_groups1,  1, count_groups("()"));
//STATIC_AUTO_TEST(count_groups2,  0, count_groups("\"()\""));
//STATIC_AUTO_TEST(count_groups3,  1, count_groups("a\\((b"));

void lexer_info::constant(int token_id, std::string const& value)
{
    const std::string regexp = "\\Q" + value + "\\E";
    const entry e = { token_id, regexp, 0 };
    entries.push_back(e);
}

void lexer_info::token(int token_id, std::string const& regexp)
{
    const int group_count = count_groups(regexp);
    const entry e = { token_id, regexp, group_count };
    entries.push_back(e);
}

void lexer_info::ignore(std::string const& regexp)
{
    const int group_count = count_groups(regexp);
    const entry e = { -1, regexp, group_count };
    entries.push_back(e);
}

//
// string_lexer implementation
// 

// static
std::string string_lexer::create_regexp(lexer_info const& li)
{
    std::ostringstream s;
    s << "^(";
    for( int i = 0; i < li.entries.size(); ++i ) {
        lexer_info::entry const& e = li.entries[i];
        if( i != 0 )
            s << "|";
        s << "(" << e.regexp << ")";
    }
    s << ")";
    return s.str();
}


bool string_lexer::next(string_lexer::token& tok)
{
    const tinfra::tstring input2(input);
    bool repeat = true;
    while( repeat ) {
        repeat = false;
        const tinfra::tstring searched_string = input2.substr(pos);
        TINFRA_TRACE_VAR(pos);
        TINFRA_TRACE_VAR(searched_string);
        
        if( searched_string.size() == 0 ) {
            TINFRA_TRACE_MSG("EOF");
            return false;
        }
        tinfra::tstring_match_result match_result;
        
        if( !re.matches(searched_string, match_result)) {
            throw std::logic_error("invalid input");
        }
        
        // iterate through match groups and entries
        // second group contains entries
        for( int ig = 2,ie = 0; 
             ig < match_result.groups.size(); 
             ++ig,++ie ) 
        {
            tinfra::tstring const&   match = match_result.groups[ig];
            lexer_info::entry const& entry = rules.entries[ie];
            // each "entry" can contain more than one group
            // so adapt group iterator for next 'entry' group
            ig += entry.regexp_groups;
            if( match.size() == 0) 
                continue;
            pos += match.size();
            
            TINFRA_TRACE_VAR(match);
            
            
            if( entry.token_id == -1 ) {
                TINFRA_TRACE_MSG("ignored");
                repeat = true;
            } else {
                TINFRA_TRACE_VAR(entry.token_id);
                tok.token_id = entry.token_id;
                tok.text = match;
                return true;
            }
        }
    }
    assert(false);
}
