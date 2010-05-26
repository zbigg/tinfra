#include "lr_parser.h"
#include "lr_parser_detail.h"

#include "tinfra/fmt.h"
#include "tinfra/trace.h"

#include <stdexcept>
#include <cassert>
#include <stack>
#include <algorithm>

using tinfra::fmt;

namespace tinfra {
namespace lr_parser {

//
// rule_list functions
//
void add_rule(rule_list& rules, symbol out, symbol in) {
    rule r;
    r.output = out;
    r.inputs.push_back(in);
    
    rules.push_back(r);
}

void add_rule(rule_list& rules, symbol out, symbol in1, symbol in2) {
    rule r;
    r.output = out;
    r.inputs.push_back(in1);
    r.inputs.push_back(in2);
    
    rules.push_back(r);
}

void add_rule(rule_list& rules, symbol out, symbol in1, symbol in2, symbol in3) {
    rule r;
    r.output = out;
    r.inputs.push_back(in1);
    r.inputs.push_back(in2);
    r.inputs.push_back(in3);
    
    rules.push_back(r);
}

bool is_terminal(rule_list const& rules, symbol sym)
{
    for( rule_list::const_iterator i = rules.begin(); i != rules.end(); ++i)
    {
        if( i->output == sym ) 
            return false;
    }
    return true;
}

//
// parser_table functions
//

parser_table generate_table(rule_list const& rules)
{
    using tinfra::lr_parser::generator::item;
    using tinfra::lr_parser::generator::item_set;
    using tinfra::lr_parser::generator::close_item_set;
    
    parser_table result_table;
    
    std::vector<item_set> item_sets;
    
    
    item_set initial;
    initial.insert(item({0,0}));
    close_item_set(rules, initial);
    
    unsigned current_state = 0;
    item_sets.push_back(initial);
    
    while( current_state < item_sets.size() ) {
        std::map<symbol, item_set> new_item_sets;
        {
            item_set const& current = item_sets[current_state];
            for(item_set::const_iterator ic = current.begin(); ic != current.end(); ++ic ) {
                item const& i = *ic;
                rule const& r = rules[i.rule_idx];
                if( i.position >= r.inputs.size() )
                    continue; // we don't take rules "with dot at end"
                symbol sym_at_dot = r.inputs[i.position];
                item ni(i);
                ni.position++;
                new_item_sets[sym_at_dot].insert(ni);
            }
        }
        if( new_item_sets.empty() ) {
            current_state++;
            continue;
        }

        for( std::map<symbol, item_set>::const_iterator inis = new_item_sets.begin();
             inis != new_item_sets.end(); ++inis )
        {
            int input           = inis->first;
            item_set nis        = inis->second;
            close_item_set(rules, nis);
            unsigned nis_state = item_sets.size();
            item_sets.push_back(nis);
            if( is_terminal(rules, input) ) {
                TINFRA_TRACE_MSG(tinfra::fmt("(%s,%s)->shift(%s)") % current_state % input % nis_state);
                result_table.add_shift(current_state, input, nis_state);
            } else {
                TINFRA_TRACE_MSG(tinfra::fmt("(%s,%s)->goto(%s)") % current_state % input % nis_state);
                result_table.add_goto(current_state, input, nis_state);
            }
            
            if( nis.find(item{0, 1}) != nis.end() ) {
                TINFRA_TRACE_MSG(tinfra::fmt("(%s,EOI)->accept") % nis_state);
                result_table.add_action(nis_state, parser_table::END_OF_INPUT,  parser_table::ACCEPT, 0);
            }
            
            if( nis.size() == 1 ) {
                item const& ni = * nis.begin();
                if( ni.rule_idx > 0 && 
                    rules[ni.rule_idx].inputs.size() == ni.position )
                {
                    TINFRA_TRACE_MSG(tinfra::fmt("(%s)->reduce(%s)") % nis_state % ni.rule_idx);
                    result_table.add_reduce(nis_state, ni.rule_idx);
                }
            }
                
        }
        current_state++;
    }
    
    return result_table;
}

//
// parsers
//

parser::parser(rule_list const& rules, 
      parser_table const& table): 
    rules(rules),
    table(table)
{
    stack.push(stack_element(0, parser_table::END_OF_INPUT));
}
parser::~parser()
{
}

void parser::operator()(symbol input)
{
    while( true ) {
        const int S = stack.top().state;
        TINFRA_TRACE_VAR(S);
        TINFRA_TRACE_VAR(input);
        parser_table::action A = table.get_action(S, input);
        TINFRA_TRACE_VAR(A);
        
        if( A.type == parser_table::REDUCE ) {
            rule const& R = rules[A.param];
            TINFRA_TRACE_MSG("reducing");
            for( size_t i = 0; i < R.inputs.size(); ++i ) {
                symbol sym = stack.top().input;
                TINFRA_TRACE_VAR(sym);
                stack.pop();
            }
            TINFRA_TRACE_VAR(R.output);
            
            const int TS = stack.top().state;
            TINFRA_TRACE_VAR(TS);
            int new_state = table.get_goto(TS, R.output);
            stack.push(stack_element(new_state, R.output));
            continue;
        }
        if( A.type == parser_table::SHIFT ) {
            stack.push(stack_element(A.param, input));
            return;
        }
        if( A.type == parser_table::ACCEPT ) {
            return;
        }
        assert(false);
    }
}

void check_syntax(std::vector<int> const& input, 
                  rule_list const& rules, 
                  parser_table const& table)
{
    parser P(rules, table);
    std::for_each(input.begin(), input.end(), P);
}

std::ostream& operator <<(std::ostream& s, parser_table::action const& a) 
{
    s << (a.type == parser_table::SHIFT  ? "shift"  :
          a.type == parser_table::REDUCE ? "reduce" :
                             "accept" );
    if( a.type != parser_table::ACCEPT ) {
        s << '(' << a.param << ')';
    }
    return s;
}

std::ostream& operator <<(std::ostream& s, parser_table::table_key const& a)
{
    return s << "(" << a.state << ", " << a.input << ")";
}

}} // end namespace tinfra::lr_parser
