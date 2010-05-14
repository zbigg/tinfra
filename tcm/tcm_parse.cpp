#include "lexer.h"
#include "parser.h"

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
    TYPE,
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
    
    string_lexer L(li, argv[1]);
    string_lexer::token tok;
    
    //parser p(rules);
    while( L.next(tok) ) 
        std::cout << "token: " << tok.text << "\n";
    
    return 0;
}

TINFRA_MAIN(tcm_parse_main);
