//
// Copyright (c) 2013, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_json_h_included
#define tinfra_json_h_included

#include "tstring.h"
#include "variant.h"
#include "generator.h"
#include "assert.h"

#include <memory>
#include <stack>

namespace tinfra {

class input_stream;
class output_stream;

enum json_encoding {
    UTF8,
    UTF16_BE,
    UTF16_LE,
    UTF32_BE,
    UTF32_LE
};

//
// json writing and parsing convienience functions
//
variant     json_parse(tstring const& s);
variant     json_parse(tinfra::input_stream& s);

void        json_write(variant const& v, tinfra::output_stream& out, json_encoding = UTF8);
std::string json_write(variant const& v, json_encoding = UTF8);

//
// json writing and parsing classes
//

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
        TOK_TRUE,
        TOK_FALSE,
        TOK_NULL
    };
    
    token_type type;
    tstring    value;
};

std::ostream& operator <<(std::ostream& s, json_token::token_type tt);

class json_lexer: public generator_impl<json_lexer, json_token> {
public:
    json_lexer(tinfra::input_stream& s);
    ~json_lexer();

    bool fetch_next(json_token&);
    
private:
    struct internal_data;
    std::auto_ptr<internal_data> self;
};

class json_renderer {
    tinfra::output_stream& out;
    json_encoding   enc;
public:
    json_renderer(tinfra::output_stream& out, json_encoding enc = UTF8);
    void object_begin();
    void object_end();
    void array_begin();
    void array_end();
    void comma();
    void colon();
    void string(tstring const& str);
    void integer(variant::integer_type v);
    void double_(double v);
    void boolean(bool v);
    void none();
};

class json_writer {
public:
    json_writer(json_renderer& renderer);
    ~json_writer();

    void begin_object();
    void named_begin_object(tstring const& name);

    void begin_array();
    void named_begin_array(tstring const& name);

    void end_object();
    void end_array();
    void end(); // ends current object/array

    enum current_value_type {
        NAMED,
        UNNAMED
    };
    current_value_type expected_value_kind() const;
    //  a value, valid only in array context
    template <typename T>
    void value(T const& v);

    //  a named value, valid only in object scope
    template <typename T>
    void named_value(tstring const& name, T const& v);

private:
    void value_impl(variant const& v);
    void value_impl(tstring const& value);
    void value_impl(variant::integer_type const& value);
    void value_impl(int const& value);
    void value_impl(double value);
    void value_impl(bool value);
    void value_impl(); // none/nil

    template <typename T>
    void value_impl(std::vector<T> const& v);

    template <typename K, typename T>
    void value_impl(std::map<K,T> const& v);

    json_renderer& renderer;
    enum container_type { OBJECT, ARRAY };
    std::stack<container_type> stack;
    bool need_separator;
    
    container_type current_type() const { return this->stack.top(); }
};

//
// implementation (templates)
//

template <typename T>
void json_writer::value(T const& v)
{
    TINFRA_ASSERT(this->stack.empty() || this->current_type() == ARRAY );
    if( this->need_separator ) {
        this->renderer.comma();
    }
    this->value_impl(v);
    this->need_separator = true;
}

template <typename T>
void json_writer::named_value(tstring const& name, T const& v)
{
    TINFRA_ASSERT(this->current_type() == OBJECT);
    if( this->need_separator ) {
        this->renderer.comma();
    }
    this->renderer.string(name);
    this->renderer.colon();
    this->value_impl(v);
    this->need_separator = true;
}

template <typename T>
void json_writer::value_impl(std::vector<T> const& v)
{
    this->begin_array();
    for( typename std::vector<T>::const_iterator i = v.begin(); i != v.end(); ++i ) {
        this->value(*i);
    }
    this->end_array();
}

template <typename K, typename T>
void json_writer::value_impl(std::map<K,T> const& v)
{
    this->begin_object();
    for( typename std::map<K,T>::const_iterator i = v.begin(); i != v.end(); ++i ) {
        this->named_value(i->first, i->second);
    }
    this->end_object();
}

inline
json_writer::current_value_type json_writer::expected_value_kind() const
{
    return this->stack.empty()
        ? UNNAMED
        : this->stack.top() == OBJECT
                ? NAMED
                : UNNAMED;
}

} // end namespace tinfra

#endif
