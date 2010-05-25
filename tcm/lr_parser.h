#ifndef tinfra_lr_parser_detail_h_included
#define tinfra_lr_parser_detail_h_included

#include <set>
#include <vector>

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
    
    bool is_terminal(symbol_t sym) const
    {
        for( rule_list::const_iterator i = rules.begin(); i != rules.end(); ++i)
        {
            if( i->output == sym ) 
                return false;
        }
        return true;
    }
    
    typedef std::pair<int, int> item;
    typedef std::vector<item> item_set;
    
    void close_item_set(item_set& is) const
    {
        for( int i = 0; i < is.size(); ++i )
        {
            int position = is[i[.econd;
            rule const& rule = rules[is[i].first];
            if( rule.inputs.size() >= position )
                continue; // we don't need to expand if dot is at END of rule ?
            int symbol_after_dot = rule.inputs[position];
        }
    }

    void generate_table()
    {
        using std::make_pair;
        
        std::vector<item_set> item_sets;
        
        item_set initial;
        initial.push_back(make_pair(0,0));
        
        close_item_set(PT, initial);
    }
};

} } // end namespace tinfra::lr_parser

#endif

