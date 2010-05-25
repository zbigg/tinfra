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

namespace generator {
    
struct item {
    int rule_idx;
    int position;
    
    bool operator <(item const& other) const {
        if( this->rule_idx < other.rule_idx) 
            return true;
        if(    (this->rule_idx == other.rule_idx)
            && (this->position < other.position) )
            return true;
        return false;
    }
};
typedef std::set<item> item_set;

/// Closes an item set.
///
/// (See http://en.wikipedia.org/wiki/LR_parser#Closure_of_item_sets)
void close_item_set(rule_list const& rules, item_set& is);

}   // end namespace tinfra::lr_parser::generator
} } // end namespace tinfra::lr_parser

#endif

