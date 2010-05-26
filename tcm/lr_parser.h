#ifndef tinfra_lr_parser_h_included
#define tinfra_lr_parser_h_included

#include <map>
#include <vector>
#include <iosfwd>
#include <stack>

#include <stdexcept>
#include "tinfra/fmt.h"

namespace tinfra {
namespace lr_parser {

/// Identifies terminal/nonterminal.
typedef int symbol;
    
struct rule {
    symbol              output;
    std::vector<symbol> inputs;
};

typedef std::vector<rule> rule_list;

/// Checks if given symbol denotes terminal in given rule set.
bool is_terminal(rule_list const& rules, symbol sym);

/// Helper for adding rules.
void add_rule(rule_list& rules, symbol out, symbol in);
void add_rule(rule_list& rules, symbol out, symbol in1, symbol in2);
void add_rule(rule_list& rules, symbol out, symbol in1, symbol in2, symbol in3);

struct parser_table {

    enum special_inputs {
        ANY_INPUT = -1,
        END_OF_INPUT = -2
    };
    
    enum parser_action_type {
        SHIFT,
        REDUCE,
        ACCEPT
    };

    struct action {
        parser_action_type type;
        int                param;
    };
    
    struct table_key {
        int      state;
        symbol   input;
        
        bool operator <(table_key const& other) const {
            if( this->state < other.state ) 
                return true;
            if(    (this->state == other.state)
                && (this->input < other.input) )
                return true;
            return false;
        }
    };
    
    typedef std::map<table_key, action> action_table;
    typedef std::map<table_key, int>    goto_table;
    
    action_table actions;
    goto_table   gotos;
    
    table_key make_key(int state, symbol sym) {
        const table_key k = { state, sym };
        return k;
    }
    
    void add_action(int state, symbol input , parser_action_type type, int param) {
        table_key k = { state, input };
        action    a = { type, param };
        
        actions[k] = a;
    }
 
    void add_shift(int state, symbol input, int new_state) {
        add_action(state, input, SHIFT, new_state);
    }
    
    void add_reduce(int state, int new_state) {
        add_action(state, ANY_INPUT, REDUCE, new_state);
    }
    
    void add_goto(int state, symbol input, int param) {
        table_key k = { state, input };
        
        gotos[k] = param;
    }    
    
    action get_action(int state, symbol input) const
    {
        {
            table_key k = { state, input };
            typename action_table::const_iterator i = actions.find(k);
            if( i != actions.end() )
                return i->second;
        }
        
        {
            table_key k = { state, ANY_INPUT };
            
            typename action_table::const_iterator i = actions.find(k);
            if( i != actions.end() )
                return i->second;
        }
        throw std::logic_error(
            tinfra::fmt("invalid action: state=%s, input=%s") 
                % state % input);
    }
    
    int get_goto(int state, symbol input) const
    {
        table_key k = { state, input };
        typename goto_table::const_iterator i = gotos.find(k);
        if( i != gotos.end() )
            return i->second;
        throw std::logic_error(
            tinfra::fmt("invalid goto: state=%s, input=%s") 
                % state % input);
    }
};

namespace detail {
    struct stack_element {
        int     state;
        symbol  input;
        
        stack_element(int st, symbol in):
            state(st),
            input(in)
        {}
};
}

class parser {
public:
    parser(rule_list const& rules, 
           parser_table const& table);
    ~parser();

    void operator()(symbol input);
private:
    typedef detail::stack_element stack_element;

    rule_list const& rules;
    parser_table const& table;
    std::stack<stack_element> stack;
};
parser_table generate_table(rule_list const& rules);

/// Checks syntax of input.
///
/// Invokes parsing algorithm without executing any rules, 
/// and actually looking at meaning of readed tokens.
///
/// Checks only syntax, not-semantics.
void check_syntax(std::vector<symbol> const& input, 
                  rule_list const&           rules, 
                  parser_table const&        table);


std::ostream& operator <<(std::ostream& s, parser_table::action const& a);


} } // end namespace tinfra::lr_parser

#endif

