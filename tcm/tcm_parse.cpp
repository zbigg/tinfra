#include <tinfra/regexp.h>
#include <tinfra/cmd.h>
#include <tinfra/trace.h>
#include <tinfra/tstring.h>

#include <vector>
#include <string>
#include <sstream>
#include <cassert>
#include <iostream>
//
// lexer_info interface
//

class lexer_info {
public:
    void constant(int token_id, std::string const& value);
    void token(int token_id, std::string const& regexp);
    void ignore(std::string const& regexp);
    
    struct entry {
        int         token_id;
        std::string regexp;
        int         regexp_groups;
    };

    std::vector<entry> entries;
};

//
// lexer_info implementation
//

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

class lexer {
public:
    lexer(lexer_info const& li, std::string const& input):
        rules(li),
        re(create_regexp(li).c_str()),
        input(input),
        pos(0)
    {
    }
    
    struct token {
        int             token_id;
        tinfra::tstring text;
    };
    
    bool next(token& tok);
private:
    
    static std::string create_regexp(lexer_info const& li);
    
    lexer_info               rules;
    tinfra::regexp           re;
    
    std::string              input;
    size_t                   pos;
};

// static
std::string lexer::create_regexp(lexer_info const& li)
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

bool lexer::next(lexer::token& tok)
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
    throw std::logic_error("FOo?");
}

///
/// parser_rules
///

struct parser_rules {
    void add(int p, int a1, int a2 = -1, int a3 = -1, int a4 = -1, int a5 = -1)
    {
        production prod;
        prod.terminal = p;
        prod.inputs.push_back(a1);
        if( a2 ) prod.inputs.push_back(a2);
        if( a3 ) prod.inputs.push_back(a3);
        if( a4 ) prod.inputs.push_back(a4);
        if( a5 ) prod.inputs.push_back(a5);
        
        productions.push_back(prod);
    }
    
    typedef std::vector<int> list;
    
    struct production {
        int   terminal;
        list  inputs;
    };
    
    std::vector<production> productions;
};

class parser {
public:
    parser(parser_rules const& prules):
        rules(prules)
    {
        check_rules();
    }
    
    
private:
    parser_rules rules;
};

enum {
    MODULE      = 0,
    STRUCT      = 1,
    ENUM        = 2,
    OPEN_BRACE  = 3,
    CLOSE_BRACE = 4,
    LESS_THAN   = 5,
    GREAT_THAN  = 6,
    COLON       = 7,
    SEMICOLON   = 8,
    IDENTIFIER  = 9,
    STRING_LITERAL = 10,
    
    COMPILATION_UNIT,
    DEFINITIONS,
    DEFINITION,
    STRUCT_DEFINITION,
    ENUM_DEFINITION,
    STRUCT_MEMBERS,
    STRUCT_MEMBER,
    TYPE,
    ENUM_MEMBERS
};

int tcm_parse_main(int argc, char** argv)
{
    
    assert( count_groups("()") == 1);
    assert( count_groups("\"()\"") == 0);
    assert( count_groups("a\\((b") == 1);
    
    lexer_info li;
    
    li.constant(MODULE,        "module");
    li.constant(STRUCT,        "struct");
    li.constant(ENUM,          "enum");
    li.constant(OPEN_BRACE,    "{");
    li.constant(CLOSE_BRACE,   "}");
    li.constant(LESS_THAN,     "<");
    li.constant(GREAT_THAN,    ">");
    li.constant(COLON,         ",");
    li.constant(SEMICOLON,     ";");
    
    li.token(IDENTIFIER,      "[a-zA-Z_][a-zA-Z_0-9]*");
    li.token(STRING_LITERAL,  "\\\"([^\\\"\\\\]|\\\\.)*\\\"");
    
    li.ignore("[ \t\r\n]+");
    
    lexer L(li,argv[1]);
    lexer::token tok;
    
    parser_rules rules;
    {
        rules.add(COMPILATION_UNIT, MODULE, IDENTIFIER, OPEN_BRACE, DEFINITIONS, CLOSE_BRACE);
        
        rules.add(DEFINITIONS,       DEFINITION);
        rules.add(DEFINITIONS,       DEFINITIONS, DEFINITION);
        
        rules.add(DEFINITION,        STRUCT_DEFINITION);
        rules.add(DEFINITION,        ENUM_DEFINITION);
        
        rules.add(STRUCT_DEFINITION, STRUCT, IDENTIFIER, OPEN_BRACE, STRUCT_MEMBERS, CLOSE_BRACE);
        
        rules.add(STRUCT_MEMBERS,    STRUCT_MEMBER);
        rules.add(STRUCT_MEMBERS,    STRUCT_MEMBERS, SEMICOLON, STRUCT_MEMBER);
        
        rules.add(STRUCT_MEMBER,     TYPE, IDENTIFIER);
        
        rules.add(TYPE,              IDENTIFIER);
        rules.add(TYPE,              IDENTIFIER, LESS_THAN, TYPE, GREAT_THAN);
        
        rules.add(ENUM_DEFINITION,   ENUM, OPEN_BRACE, ENUM_MEMBERS, CLOSE_BRACE);
        rules.add(ENUM_MEMBERS,      IDENTIFIER);
        rules.add(ENUM_MEMBERS,      ENUM_MEMBERS, COLON, IDENTIFIER  );
    }
    {
    }
    
    parser p(rules);
    while( L.next(tok) ) 
        std::cout << "token: " << tok.text << "\n";
    
    return 0;
}

TINFRA_MAIN(tcm_parse_main);
