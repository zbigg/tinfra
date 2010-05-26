#ifndef tinfra_lr_parser_detail_h_included
#define tinfra_lr_parser_detail_h_included

#include <set>
#include <vector>

namespace tinfra {
namespace lr_parser {
namespace generator {
    
struct item {
    unsigned int rule_idx;
    unsigned int position;
    
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

} } } // end namespace tinfra::lr_parser::generator

#endif

