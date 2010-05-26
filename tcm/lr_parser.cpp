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
    
    parser_table result;
    
    std::vector<item_set> item_sets;
        
    item_set initial;
    initial.insert(item({0,0}));
    
    close_item_set(rules, initial);
    
    return result;
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
