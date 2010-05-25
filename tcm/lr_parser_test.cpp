#include <unittest++/UnitTest++.h>
#include "tinfra/test.h"
#include "tinfra/cmd.h"
#include "lr_parser_detail.h"

using namespace tinfra::lr_parser;

enum test_grammar {
    GRAMMAR  = 0,
    ZERO     = 1,
    ONE      = 2,
    PLUS     = 3,
    STAR     = 4,
    E        = 5,
    B        = 6,
    EOI
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

template <typename T>
bool contains(std::set<T> const& s, T const& k)
{
    return s.find(k) != s.end();
}

TEST(lr_parser_close_item_set)
{
    using generator::item;
    rule_list rules = get_test_grammar();
    generator::item_set iset;
    iset.insert(item({ GRAMMAR, 0 }));
    
    generator::close_item_set(rules, iset);
    
    CHECK( contains(iset, item({ 0, 0 })));
    CHECK( contains(iset, item({ 1, 0 })));
    CHECK( contains(iset, item({ 2, 0 })));
    CHECK( contains(iset, item({ 3, 0 })));
    CHECK( contains(iset, item({ 4, 0 })));
    CHECK( contains(iset, item({ 5, 0 })));
    CHECK_EQUAL(6, iset.size());
}

TINFRA_MAIN(tinfra::test::test_main);
