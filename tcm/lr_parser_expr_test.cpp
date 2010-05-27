#include <unittest++/UnitTest++.h>
#include "tinfra/test.h"
#include "lr_parser.h"

using namespace tinfra::lr_parser;

enum test_grammar {
    GRAMMAR  = 0,
    NUMBER   = 1,
    VARIABLE = 2,
    PLUS     = 3,
    STAR     = 4,
    OPEN_BRACE = 5,
    CLOSE_BRACE = 6,
    E        = 7,
    B        = 8
};

static rule_list get_test_grammar() {
    rule_list G;
    add_rule(G, GRAMMAR, E);
    add_rule(G, E, E, STAR, B);
    add_rule(G, E, E, PLUS, B);
    add_rule(G, E, OPEN_BRACE, E, CLOSE_BRACE);
    add_rule(G, E, B);
    add_rule(G, B, NUMBER);
    add_rule(G, B, VARIABLE);
    return G;
}


TEST(lr_parser_expr_generate_table)
{
    rule_list rules = get_test_grammar();
    parser_table generated_table = generate_table(rules);
    parser P(rules, generated_table);
    
    P(NUMBER);
    P(parser_table::END_OF_INPUT);
}


TEST(lr_parser_expr_first)
{
    rule_list rules = get_test_grammar();
    parser_table generated_table = generate_table(rules);
    parser P(rules, generated_table);
    
    P(OPEN_BRACE);
    P(NUMBER);
    P(CLOSE_BRACE);
    P(PLUS);
    P(VARIABLE);
    P(parser_table::END_OF_INPUT);
}
