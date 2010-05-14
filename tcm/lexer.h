#ifndef tinfra_lexer_h_included
#define tinfra_lexer_h_included

#include <tinfra/regexp.h>

#include <vector>
#include <string>

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

class string_lexer {
public:
    string_lexer(lexer_info const& li, std::string const& input):
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

#endif
