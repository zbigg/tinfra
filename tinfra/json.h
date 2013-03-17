//
// Copyright (c) 2013, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_json_h_included
#define tinfra_json_h_included

#include "tstring.h"
#include "variant.h"
#include "generator.h"

#include <memory>

namespace tinfra {

class input_stream;
class output_stream;

struct json_token {
    enum token_type {
        OBJECT_BEGIN,
        OBJECT_END,
        ARRAY_BEGIN,
        ARRAY_END,
        COMMA,
        COLON,
        STRING,
        INTEGER,
        DOUBLE,
        TRUE,
        FALSE,
        TOK_NULL
    };
    
    token_type type;
    tstring    value;
};

class json_lexer: public generator_impl<json_lexer, json_token> {
public:
    json_lexer(tinfra::input_stream& s);
    ~json_lexer();

    bool fetch_next(json_token&);
    
private:
    struct internal_data;
    std::auto_ptr<internal_data> self;
};

enum json_encoding {
    UTF8,
    UTF16_BE,
    UTF16_LE,
    UTF32_BE,
    UTF32_LE
};

variant     json_parse(tstring const& s);
variant     json_parse(tinfra::input_stream& s);

void        json_write(variant const& v, tinfra::output_stream& out, json_encoding = UTF8);
std::string json_write(variant const& v, json_encoding = UTF8);

} // end namespace tinfra

#endif
