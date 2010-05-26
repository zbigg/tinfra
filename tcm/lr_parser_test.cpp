#include <unittest++/UnitTest++.h>
#include "tinfra/test.h"
#include "tinfra/cmd.h"
#include "lr_parser.h"
#include "lr_parser_detail.h"

using namespace tinfra::lr_parser;

enum test_grammar {
    GRAMMAR  = 0,
    ZERO     = 1,
    ONE      = 2,
    PLUS     = 3,
    STAR     = 4,
    E        = 5,
    B        = 6
};

rule_list get_test_grammar() {
    rule_list G;
    add_rule(G, GRAMMAR, E);
    add_rule(G, E, E, STAR, B);
    add_rule(G, E, E, PLUS, B);
    add_rule(G, E, B);
    add_rule(G, B, ZERO);
    add_rule(G, B, ONE);
    return G;
}

parser_table get_test_parser_table()
{
    parser_table table;
    table.add_shift (0, ZERO, 1);
    table.add_shift (0, ONE,  2);
    table.add_shift (3, STAR, 5);
    table.add_shift (3, PLUS, 6);
    table.add_action(3, parser_table::END_OF_INPUT,  parser_table::ACCEPT, 0);
    table.add_shift (5, ZERO, 1);
    table.add_shift (5, ONE,  2);
    table.add_shift (6, ZERO, 1);
    table.add_shift (6, ONE,  2);
    
    // gotos
    table.add_goto  (0, E,    3);
    table.add_goto  (0, B,    4);
    table.add_goto  (5, B,    7);
    table.add_goto  (6, B,    8);
    
    // reduces
    table.add_reduce(1, 4);
    table.add_reduce(2, 5);
    table.add_reduce(4, 3);
    table.add_reduce(7, 1);
    table.add_reduce(8, 2);
    
    return table;
}

template <typename T>
bool contains(std::set<T> const& s, T const& k)
{
    return s.find(k) != s.end();
}

TEST(lr_parser_close_item_set1)
{
    using generator::item;
    rule_list rules = get_test_grammar();
    generator::item_set iset;
    iset.insert(item({ 0, 0 }));
    
    generator::close_item_set(rules, iset);
    
    CHECK( contains(iset, item({ 0, 0 })));
    CHECK( contains(iset, item({ 1, 0 })));
    CHECK( contains(iset, item({ 2, 0 })));
    CHECK( contains(iset, item({ 3, 0 })));
    CHECK( contains(iset, item({ 4, 0 })));
    CHECK( contains(iset, item({ 5, 0 })));
    CHECK_EQUAL(6, iset.size());
}

TEST(lr_parser_close_item_set2)
{
    using generator::item;
    rule_list rules = get_test_grammar();
    generator::item_set iset;
    iset.insert(item({ 1, 2 }));
    
    generator::close_item_set(rules, iset);
    
    CHECK( contains(iset, item({ 1, 2 })));
    CHECK( contains(iset, item({ 4, 0 })));
    CHECK( contains(iset, item({ 5, 0 })));
    CHECK_EQUAL(3, iset.size());
}

TEST(lr_parser_check_syntax_basic)
{
    rule_list rules = get_test_grammar();
    parser_table table = get_test_parser_table();
    
    std::vector<int> input;
    input.push_back(ONE);
    input.push_back(PLUS);
    input.push_back(ONE);
    input.push_back(STAR);
    input.push_back(ZERO);
    input.push_back(parser_table::END_OF_INPUT);
    
    check_syntax(input, rules, table);
}

TEST(lr_parser_parse_basic)
{
    rule_list rules = get_test_grammar();
    parser_table table = get_test_parser_table();
    
    parser P(rules, table);
    P(ONE);
    P(PLUS);
    P(ONE);
    P(STAR);
    P(ZERO);
    P(parser_table::END_OF_INPUT);
}

TINFRA_MAIN(tinfra::test::test_main);
