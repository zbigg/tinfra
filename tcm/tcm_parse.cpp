#include "lexer.h"
#include "lr_parser.h"

#include <tinfra/cmd.h>
#include <iostream>


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
    COMPLEX_TYPE,
    ENUM_MEMBERS
};

int tcm_parse_main(int argc, char** argv)
{
    lexer_info li;
    {
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
    }
    
    tinfra::lr_parser::rule_list rules;
    {
        using tinfra::lr_parser::add_rule;
        add_rule(rules, COMPILATION_UNIT, MODULE, IDENTIFIER, OPEN_BRACE, DEFINITIONS, CLOSE_BRACE);
        
        add_rule(rules, DEFINITIONS,       DEFINITION);
        add_rule(rules, DEFINITIONS,       DEFINITIONS, DEFINITION);
        
        add_rule(rules, DEFINITION,        STRUCT_DEFINITION);
        add_rule(rules, DEFINITION,        ENUM_DEFINITION);
        
        add_rule(rules, STRUCT_DEFINITION, STRUCT, IDENTIFIER, OPEN_BRACE, STRUCT_MEMBERS, CLOSE_BRACE);
        
        add_rule(rules, STRUCT_MEMBERS,    STRUCT_MEMBER);
        add_rule(rules, STRUCT_MEMBERS,    STRUCT_MEMBERS, SEMICOLON, STRUCT_MEMBER);
        
        add_rule(rules, STRUCT_MEMBER,     IDENTIFIER, IDENTIFIER);
        add_rule(rules, STRUCT_MEMBER,     COMPLEX_TYPE, IDENTIFIER);
        
        add_rule(rules, COMPLEX_TYPE,      IDENTIFIER, LESS_THAN, IDENTIFIER, GREAT_THAN);
        add_rule(rules, COMPLEX_TYPE,      IDENTIFIER, LESS_THAN, COMPLEX_TYPE, GREAT_THAN);
        
        add_rule(rules, ENUM_DEFINITION,   ENUM, IDENTIFIER, OPEN_BRACE, ENUM_MEMBERS, CLOSE_BRACE);
        add_rule(rules, ENUM_MEMBERS,      IDENTIFIER);
        add_rule(rules, ENUM_MEMBERS,      ENUM_MEMBERS, COLON, IDENTIFIER  );
    }
    tinfra::lr_parser::parser_table parser_table = tinfra::lr_parser::generate_table(rules);
    
    string_lexer L(li, argv[1]);
    string_lexer::token tok;
    
    tinfra::lr_parser::parser p(rules, parser_table);
    while( L.next(tok) ) {
        std::cout << "token: " << tok.text << "(" << tok.token_id << ")" << "\n";
        p(tok.token_id);
    }
    
    return 0;
}

TINFRA_MAIN(tcm_parse_main);
