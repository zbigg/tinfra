#include "lr_parser.h"
#include "lr_parser_detail.h"

#include "tinfra/fmt.h"
#include "tinfra/trace.h"

#include <stdexcept>
#include <cassert>

using tinfra::fmt;

namespace tinfra {
namespace lr_parser {
namespace generator {

void close_item_set(rule_list const& rules, item_set& is)
{
    bool expanded;
    do {
        expanded = false;
        for( item_set::const_iterator iis = is.begin(); iis != is.end(); ++iis)
        {
            item const& current_item = *iis;
            rule const& rule = rules[current_item.rule_idx];
            if( rule.inputs.size() <= current_item.position )
                continue; // we don't need to expand if dot is at END of rule ?
            
            int symbol_after_dot = rule.inputs[current_item.position];
            
            if( is_terminal(rules, symbol_after_dot) )
                continue; // we don't need to expand, if next symbol is terminal
            
            bool found = false;
            for( rule_list::const_iterator ir = rules.begin(); ir != rules.end(); ++ir ) {
                if( ir->output != symbol_after_dot ) 
                    continue;
                int rule_idx = distance(rules.begin(), ir);
                item item_candidate = { rule_idx, 0 };
                //TINFRA_TRACE_VAR(rule_idx);
                expanded = is.insert(item_candidate).second;
                found = true;
            }
            if( !found ) {
                throw std::logic_error(
                    fmt("unable to expand item_set(%s,%s) into terminal") 
                        % current_item.rule_idx
                        % current_item.position);
            }
        }
    } while (expanded);
}

}

}} // end namespace tinfra::lr_parser
