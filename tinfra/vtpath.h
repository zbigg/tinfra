//
// Copyright (c) 2013, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_vtpath_h_included
#define tinfra_vtpath_h_included

#include "tstring.h"
#include "variant.h"
#include "generator.h"

#include <memory>

namespace tinfra {

//
// variant_tree_path
//
// XPath/JSONPath variant tree visitor
//
// target: 
//    support JSONPath language for traversal of variant trees (same model as
//    JSON) 
// JSONPath: http://goessner.net/articles/JsonPath/
//

class vtpath_visitor: public generator_impl<vtpath_visitor, variant*> {
public:
    vtpath_visitor(variant* v, tstring const& expression);
    ~vtpath_visitor();

    bool fetch_next(variant*&);
    
private:
    struct internal_data;
    std::auto_ptr<internal_data> self;
};

std::vector<const variant*> vtpath_visit(variant const& v, tstring const& expression);
std::vector<variant*> vtpath_visit(variant& v, tstring const& expression);

} // end namespace tinfra

#endif
