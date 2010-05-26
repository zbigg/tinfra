#include "lexer.h"
#include "parser.h"

#include <tinfra/cmd.h>
#include <tinfra/fmt.h>
#include <tinfra/trace.h>
#include <iostream>

#include <map>
#include <vector>
#include <stack>

enum parser_action_type {
    SHIFT,
    REDUCE,
    ACCEPT
};

struct action {
    parser_action_type type;
    int param;
};

template <class T>
struct parser_table {
    
    typedef T symbol_t;
    
    struct table_key {
        int      state;
        symbol_t symbol;
        
        bool operator <(table_key const& other) const {
            if( this->state < other.state ) 
                return true;
            if(    (this->state == other.state)
                && (this->symbol < other.symbol) )
                return true;
            return false;
        }
    };
    
    
    
    struct rule {
        symbol_t              output;
        std::vector<symbol_t> inputs;
    };
    
    typedef std::map<table_key, action> action_table;
    typedef std::map<table_key, int>    goto_table;
    typedef std::vector<rule>           rule_list;
    
    action_table actions;
    goto_table   gotos;
    rule_list    rules;
    
    table_key make_key(int state, symbol_t symbol) {
        const table_key k = { state, symbol };
        return k;
    }
    
    void add_rule(symbol_t out, symbol_t in) {
        rule r;
        r.output = out;
        r.inputs.push_back(in);
        
        rules.push_back(r);
    }
    
    void add_rule(symbol_t out, symbol_t in1, symbol_t in2) {
        rule r;
        r.output = out;
        r.inputs.push_back(in1);
        r.inputs.push_back(in2);
        
        rules.push_back(r);
    }
    
    void add_rule(symbol_t out, symbol_t in1, symbol_t in2, symbol_t in3) {
        rule r;
        r.output = out;
        r.inputs.push_back(in1);
        r.inputs.push_back(in2);
        r.inputs.push_back(in3);
        
        rules.push_back(r);
    }
    
    void add_action(int state, symbol_t input , parser_action_type type, int param) {
        table_key k = { state, input };
        action    a = { type, param };
        
        actions[k] = a;
    }
 
    void add_shift(int state, symbol_t input, int new_state) {
        add_action(state, input, SHIFT, new_state);
    }
    
    void add_reduce(int state, int new_state) {
        add_action(state, static_cast<symbol_t>(-1), REDUCE, new_state);
    }
    
    void add_goto(int state, symbol_t input, int param) {
        table_key k = { state, input };
        
        gotos[k] = param;
    }    
    
    action get_action(int state, symbol_t input) const
    {
        {
            table_key k = { state, input };
            typename action_table::const_iterator i = actions.find(k);
            if( i != actions.end() )
                return i->second;
        }
        
        {
            table_key k = { state, static_cast<symbol_t>(-1) };
            
            typename action_table::const_iterator i = actions.find(k);
            if( i != actions.end() )
                return i->second;
        }
        assert(false);
        throw std::logic_error("invalid action request");
    }
    
    int get_goto(int state, symbol_t input) const
    {
        table_key k = { state, input };
        typename goto_table::const_iterator i = gotos.find(k);
        if( i != gotos.end() )
            return i->second;
        assert(false);
        throw std::logic_error("invalid goto request");
    }
};

std::ostream& operator <<(std::ostream& s, action const& a) 
{
    s << (a.type == SHIFT  ? "shift"  :
          a.type == REDUCE ? "reduce" :
                             "accept" );
    if( a.type != ACCEPT ) {
        s << '(' << a.param << ')';
    }
    return s;
}

enum S {
    GRAMMAR  = 0,
    ZERO     = 1,
    ONE      = 2,
    PLUS     = 3,
    STAR     = 4,
    E        = 5,
    B        = 6,
    EOI
};

template <typename T>
struct state_base {
    int state;
    T   symbol;
    
    state_base(int st, T sym):
        state(st),
        symbol(sym)
    {}
};
template <typename T>
void parse(std::vector<T> const& IN, parser_table<T> const& PT)
{
    typedef state_base<T> state;
    std::stack<state>  stack;
    stack.push(state(0, EOI));
    
    typename std::vector<T>::const_iterator IIN = IN.begin();
    bool accepted = false;
    while( ! accepted && IIN != IN.end()) {
        const int S = stack.top().state;
        TINFRA_TRACE_VAR(S);
        TINFRA_TRACE_VAR(*IIN);
        action A = PT.get_action(S, *IIN);
        TINFRA_TRACE_VAR(A);
        
        if( A.type == REDUCE ) {
            typename parser_table<T>::rule R = PT.rules[A.param];
            std::cout << "(";
            TINFRA_TRACE_MSG("reducing");
            for( int i = 0; i < R.inputs.size(); ++i ) {
                T sym = stack.top().symbol;
                TINFRA_TRACE_VAR(sym);
                if( i > 0 ) std::cout << ", ";
                std::cout << sym;
                stack.pop();
            }
            std::cout << ") -> " << R.output << "\n";
            TINFRA_TRACE_VAR(R.output);
            
            const int TS = stack.top().state;
            TINFRA_TRACE_VAR(TS);
            int new_state = PT.get_goto(TS, R.output);
            stack.push(state(new_state, R.output));
            continue;
        }
        if( A.type == SHIFT ) {
            stack.push(state(A.param, *IIN));
            ++IIN;
            continue;
        }
        if( A.type == ACCEPT ) {
            accepted = true;
            continue;
        }
        assert(false);
    }
}

int calc_main(int argc, char** argv)
{
    lexer_info li;
    {
        li.constant(ZERO,       "0");
        li.constant(ONE,        "1");
        li.constant(PLUS,       "+");
        li.constant(STAR,       "*");
        
        li.ignore("[ \t\r\n]+");
    }
    
    parser_table<S> G;
    parser_table<S> F;
    {
        
        G.add_rule(GRAMMAR, E);
        G.add_rule(E, E, STAR, B);
        G.add_rule(E, E, PLUS, B);
        G.add_rule(E, B);
        
        G.add_rule(B, ZERO);
        G.add_rule(B, ONE);
        F = G;
        // shifts
        G.add_shift (0, ZERO, 1);
        G.add_shift (0, ONE,  2);
        G.add_shift (3, STAR, 5);
        G.add_shift (3, PLUS, 6);
        G.add_action(3, EOI,  ACCEPT, 0);
        G.add_shift (5, ZERO, 1);
        G.add_shift (5, ONE,  2);
        G.add_shift (6, ZERO, 1);
        G.add_shift (6, ONE,  2);
        
        // gotos
        G.add_goto  (0, E,    3);
        G.add_goto  (0, B,    4);
        G.add_goto  (5, B,    7);
        G.add_goto  (6, B,    8);
        
        // reduces
        G.add_reduce(1, 4);
        G.add_reduce(2, 5);
        G.add_reduce(4, 3);
        G.add_reduce(7, 1);
        G.add_reduce(8, 2);
    }
    
    
    string_lexer L(li, argv[1]);
    string_lexer::token tok;
    
    std::vector<S> input;
    while( L.next(tok) ) {
        std::cout << "token: " << tok.text << "\n";
        input.push_back(static_cast<S>(tok.token_id));
    }
    input.push_back(EOI);
    
    parse(input, G);
    return 0;
}

TINFRA_MAIN(calc_main);
