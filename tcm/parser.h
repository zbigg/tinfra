#ifndef tinfra_parser_h_included
#define tinfra_parser_h_included

#include <vector>

//
// parser_rules interface
//

struct parser_rules {
    void add(int p, int a1, int a2 = -1, int a3 = -1, int a4 = -1, int a5 = -1)
    {
        production prod;
        prod.output = p;
        prod.inputs.push_back(a1);
        if( a2 ) prod.inputs.push_back(a2);
        if( a3 ) prod.inputs.push_back(a3);
        if( a4 ) prod.inputs.push_back(a4);
        if( a5 ) prod.inputs.push_back(a5);
        
        productions.push_back(prod);
    }
    
    typedef std::vector<int> list;
    
    struct production {
        int   output;
        list  inputs;
    };
    
    std::vector<production> productions;
};

//
// parser interface
//

class parser {
public:
    parser(parser_rules const& prules):
        rules(prules)
    {
        //check_rules();
    }
    
    
private:
    parser_rules rules;

    // sketch of state/action and
    // other runtime structures
    struct traverse_position {
        int state;
        int production;
        int position;
    };

    struct action {
        int token;
        enum {
            SHIFT,
            REDUCE
        };
        int reduce_action;
        int next_state;
    };

    struct state {
        std::vector<action> actions;
    };
};


#endif // tinfra_parser_h_included
