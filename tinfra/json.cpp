#include "json.h" // we implement this

#include "tinfra/variant.h"
#include "tinfra/stream.h"
#include "tinfra/lex.h"
#include "tinfra/fmt.h"
#include "tinfra/string.h"
#include "tinfra/trace.h"

#include <ostream>
// impl
namespace tinfra {

std::ostream& operator <<(std::ostream& s, json_token::token_type tt)
{
    switch( tt ) {
    case json_token::OBJECT_BEGIN: s << "object begin '{'"; break;
    case json_token::OBJECT_END: s << "object end '}'"; break;
    case json_token::ARRAY_BEGIN: s << "array begin '['"; break;
    case json_token::ARRAY_END: s << "array end ']'"; break;
    case json_token::COMMA: s << "comma ','"; break;
    case json_token::COLON: s << "colon ':'"; break;
    case json_token::STRING: s << "string"; break;
    case json_token::INTEGER: s << "integer"; break;
    case json_token::DOUBLE: s << "double"; break;
    case json_token::TRUE: s << "true"; break;
    case json_token::FALSE: s << "false"; break;
    }
    return s;
}

tinfra::module_tracer json_parser_tracer(tinfra::tinfra_tracer, "json_parser");
class json_parser {
    json_lexer& lexer;
    variant&    root;
    
    json_token  current;
    bool        finished;
public:
    json_parser(json_lexer& l, variant& target):
        lexer(l),
        root(target),
        finished(false)
    {
        next();
    }

    void next() {
        if( finished )
            return;
        this->finished = !this->lexer.fetch_next(this->current);
    }
    void expect(json_token::token_type tt) {
        if( finished ) {
            throw std::runtime_error(tsprintf("epecting %s, but end of input reached",
                                              tt));
        }
        if( this->current.type != tt ) {
            throw std::runtime_error(tsprintf("epecting %s, but found %s",
                                              tt, this->current.type));
        }
    }
    void parse()
    {
        parse_node(root);
    }
    void parse_node(variant& dest)
    {
        if( finished )
            return;
        switch( current.type ) {
        case json_token::OBJECT_BEGIN:
            parse_object(dest);
            break;
        case json_token::ARRAY_BEGIN:
            parse_array(dest);
            break;
        case json_token::STRING:
            dest.set_string(current.value.str());
            next();
            break;
        case json_token::INTEGER:
            {
                variant::integer_type value = tinfra::from_string<variant::integer_type>(current.value.str());
                dest.set_integer(value);
            }
            next();
            break;
        case json_token::DOUBLE:
            {
                double value = tinfra::from_string<double>(current.value.str());
                dest.set_double(value);
            }
            next();
            break;
        }
    }
    void parse_object(variant& dict)
    {
        expect(json_token::OBJECT_BEGIN);
        next();
        if( current.type == json_token::OBJECT_END ) {
            // short circuit for {}
            next();
            return;
        }
        
        while( true ) {
            // 'foo'
            expect(json_token::STRING);
            std::string key(current.value.str());
            next();
            // 'foo' :
            expect(json_token::COLON);
            
            {
                // 'foo' : ANYTHING
                variant tmp;
                parse_node(tmp);
                
                using std::swap;
                swap(dict[key],tmp);
            }
            
            // now decide: next or end
            if( current.type == json_token::COMMA ) {
                // , -> continue
                next();
                continue;
            }
            if( current.type == json_token::OBJECT_END ) {
                // } -> break
                next();
                break;
            }
        }
    }
    
    void parse_array(variant& array)
    {
        expect(json_token::ARRAY_BEGIN);
        next();
        if( current.type == json_token::ARRAY_END ) {
            // short circuit for []
            next();
            return;
        }
        
        while( true ) {
            {
                // ANYTHING
                variant tmp;
                parse_node(tmp);
                
                using std::swap;
                swap(array[array.size()],tmp);
            }
            
            // now decide: next or end
            if( current.type == json_token::COMMA ) {
                // , -> continue
                next();
                continue;
            }
            if( current.type == json_token::ARRAY_END ) {
                // ] -> break
                next();
                break;
            }
        }
    }
};
variant json_parse(tstring const& s)
{
    std::auto_ptr<input_stream> stream(create_memory_input_stream(s.data(), s.size(),USE_BUFFER));
    
    return json_parse(*stream);
}

variant json_parse(tinfra::input_stream& in)
{
    json_lexer lexer(in);
    variant result;
    json_parser parser(lexer, result);
    
    parser.parse();
    return result;
}

tinfra::module_tracer json_writer_tracer(tinfra::tinfra_tracer, "json_writer");

class json_writer {
    tinfra::output_stream& out;
    json_encoding   enc;
public:
    json_writer(tinfra::output_stream& out, json_encoding enc): out(out),enc(enc) {}
    void object_begin() {
        this->out.write("{",1);
    }
    void object_end() {
        this->out.write("}",1);
    }
    void array_begin() {
        this->out.write("[",1);
    }
    void array_end() {
        this->out.write("[",1);
    }
    void comma() {
        this->out.write(",",1);
    }
    void colon() {
        this->out.write(":",1);
    }
    void string(tstring const& str)
    {
        std::string escaped = escape_c(str);
        this->out.write("\"",1);
        this->out.write(escaped);
        this->out.write("\"",1);
    }
    void integer(variant::integer_type v)
    {
        std::string formatted = tinfra::to_string(v);
        this->out.write(formatted);
    }
    void double_(double v)
    {
        std::string formatted = tinfra::to_string(v);
        this->out.write(formatted);
    }
    void boolean(bool v)
    {
        if( v ) {
            this->out.write("true");
        } else {
            this->out.write("false");
        }
    }
    void none()
    {
        this->out.write("nil");
    }
    void node(variant const& v)
    {
        if( v.is_dict() ) {
            this->object_begin();
            variant::dict_type const& dict = v.get_dict();
            for( variant::dict_type::const_iterator i = dict.begin(); i != dict.end(); ++i ) 
            {
                if( i != dict.begin() )
                    this->comma();
                this->string(i->first);
                this->colon();
                this->node(i->second);
            }
            this->object_end();
        } else if( v.is_array() ) {
            this->array_begin();
            variant::array_type const& array = v.get_array();
            for( variant::array_type::const_iterator i = array.begin(); i != array.end(); ++i ) {
                if( i != array.begin() )
                    this->comma();
                this->node(*i);
            }
            this->array_end();
        } else if( v.is_string() ) {
            this->string(v.get_string());
        } else if( v.is_integer() ) {
            this->integer(v.get_integer());
        } else if ( v.is_double() ) {
            this->double_(v.get_double());
        } else if ( v.is_none() ) {
            this->none();
        }
        //} else if ( v.is_boolean() ) {
        //    this->boolean(v.get_boolean());
        //}
    }
};
void        json_write(variant const& v, tinfra::output_stream& out, json_encoding encoding)
{
    json_writer writer(out, encoding);
    writer.node(v);
}

std::string json_write(variant const& v, json_encoding encoding)
{
    std::string result;
    std::auto_ptr<output_stream> stream(create_memory_output_stream(result));
    
    json_write(v, *stream, encoding);
    //stream->flush();
    return result;
}

//
// json_lexer
//

tinfra::module_tracer json_lexer_tracer(tinfra::tinfra_tracer, "json_lexer");

struct json_lexer::internal_data {
    int   current;
    bool  finished;
    
    input_stream* input;
    json_encoding input_encoding;
    enum {
        BUFFER_LEN = 10
    };
    char*   buffer_start;
    char*   buffer_end;
    char    buffer[BUFFER_LEN];
    
    std::string last_token;
    
    int    bytes_in_buffer() {
        return buffer_end - buffer_start;
    }
    
    bool fill_buffer(size_t required_size_at_least) {
        const size_t current_len_at_start = this->bytes_in_buffer();
        
        if( current_len_at_start >= required_size_at_least )
            return true;
        if( finished ) {
            return false;
        }
        
        TINFRA_ASSERT(required_size_at_least < BUFFER_LEN);
        if( buffer_start > buffer ) {
            memmove(buffer, buffer_start, current_len_at_start);
            buffer_start = buffer;
            buffer_end   = buffer+current_len_at_start;
        }
        const size_t len_required = required_size_at_least - current_len_at_start;
        int r = read_for_sure(buffer_end, len_required);
        this->buffer_end += r;
        
        if( r == 0 && bytes_in_buffer() == 0 ) {
            this->finished = true;
        }
        return bytes_in_buffer() >= required_size_at_least;
    }
    
    int read_for_sure(void* buf, size_t len)
    {
        char* buf2 = static_cast<char*>(buf);
        size_t readed = 0;
        while( len > 0 ) {
            int r = this->input->read(buf2, len);
            if( r == 0 ) {
                return readed;
            }
            readed += r;
            len -= r;
            buf2 += r;
        }
        return readed;
    }
    
    bool next() {
        switch( this->input_encoding ) {
        case UTF8:
            return next_utf8();
        default:
            throw std::runtime_error(tsprintf("json_lexer: encoding %s not implemented", (int)this->input_encoding));
        }
    }
    
    bool next_utf8() {
        // ok, this is fake, we just return byte by byte
        if( !fill_buffer(1) )
            return false;
        this->current = * (this->buffer_start);
        this->buffer_start += 1;
        return true;
    }
    
    void detect_encoding()
    {
        int r =  this->read_for_sure(this->buffer, 4);
        this->buffer_start = buffer;
        this->buffer_end   = buffer + r;
        
        switch( r ) {
        case 0: // 0-byte JSON,
            this->finished = true;
            break;
        case 1: 
            // only 1-byte JSON, so it can be actually only 1-digit integer
            this->input_encoding = UTF8;
            break;
        case 2:
        case 3:
            // only 2- or 3-byte JSON, so it can be 1 UTF-8 or invalid UTF16
            if( this->buffer[0] == 0 ) {
                this->input_encoding = UTF16_BE;
            } else if( this->buffer[1] == 0 ) {
                this->input_encoding = UTF16_BE;
            } else {
                this->input_encoding = UTF8;
            }
            break;
        case 4:
            if( this->buffer[0] == 0 ) { // 00 ... 
                if (this->buffer[1] == 0 ) { // 00 00 ...
                    this->input_encoding = UTF32_BE;
                } else { // 00 xx ...
                    this->input_encoding = UTF16_BE;
                }
            } else { // xx ...
                if (this->buffer[1] == 0 ) { // xx 00 ...
                    if( this->buffer[2] == 0 ) { // xx 00 00 ..
                        this->input_encoding = UTF32_LE;
                    } else { // xx 00 xx ...
                        this->input_encoding = UTF16_LE;
                    }
                } else { // xx xx ...
                    this->input_encoding = UTF8;
                }
            }
            break;
        }
    }
    void fail(std::string const& message)
    {
        throw std::runtime_error(tsprintf("json_lexer: %s", message));
    }
    int parse_hexdigit(int c) {
        switch( c ) {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        case 'a': case 'A': return 10;
        case 'b': case 'B': return 11;
        case 'c': case 'C': return 12;
        case 'd': case 'D': return 13;
        case 'e': case 'E': return 14;
        case 'f': case 'F': return 15;
        }
        return 0;
    }
    void consume_string()
        // assuming that current == ' or "
        // parse string and put it into this->last_token
    {
        last_token = "";
        TINFRA_ASSERT(current == '"');
        next(); // skip the "
        while( true ) {
            switch(this->current) {
            case '"': // just break
                next();
                TINFRA_TRACE(json_lexer_tracer, "readed STRING(" << last_token << ")");
                return;
            case '\\':
                if( !next() ) {
                    fail("'\\' at end of input");
                }
                switch(this->current) {
                case '"':  last_token.append('"',1); break;
                case '\\': last_token.append('\\',1); break;
                case '/':  last_token.append('/',1); break;
                case 'b':  last_token.append('\b',1); break;
                case 'f':  last_token.append('\f',1); break;
                case 'n':  last_token.append('\b',1); break;
                case 'r':  last_token.append('\r',1); break;
                case 't':  last_token.append('\t',1); break;
                case 'u':
                    // TBD. parse unicode
                    {
                        int result_char = 0;
                        for(int i = 0; i < 4; ++i) {
                            if( !next() ) {
                                fail("invalid '\\uXXXX' expression, expecting 4 hex digits but EOF reached");
                            }
                            if( !isxdigit(this->current)) {
                                fail("invalid '\\uXXXX' expression, expecting 4 hex digits, non-hex digit found");
                            }
                            int n = parse_hexdigit(this->current);
                            result_char = (result_char << 8) | n;
                        }
                        if( result_char > 127) fail("we don't support anything plain old ASCII");
                        last_token.append((char)result_char,1);
                    }
                    break;
                }
                next();
                break;
            default:
                if( this->current > 127) fail("we don't support anything plain old ASCII");
                last_token.append((char)this->current,1);
                next();
                break;
            }
        } // end while
    }
    
    json_token::token_type consume_number()
        // assuming that current is first digit of number
        // parse numer and put it into this->last_token
        // and return token type (DOUBLE | INTEGER)
    {
        json_token::token_type token_type = json_token::INTEGER;
        last_token = "";
        while( !finished ) {
            switch( this->current ) {
            case 'e': case 'E':
            case '.':
                token_type = json_token::DOUBLE;
                last_token.append((char)this->current, 1);
                next();
                break;
            case '-':
            case '+':
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9': // numbers: integer or double
                last_token.append((char)this->current, 1);
                next();
                break;
            default: // any other character means end of token
                TINFRA_TRACE(json_lexer_tracer, "readed " << 
                                                (token_type == json_token::INTEGER ? "INTEGER": "DOUBLE") << 
                                                "(" << last_token << ")");
                break;
            }
        }
        TINFRA_TRACE(json_lexer_tracer, "readed " << 
                                        (token_type == json_token::INTEGER ? "INTEGER": "DOUBLE") << 
                                        "(" << last_token << ")");
    }
    
    void consume_keyword(const char* keyword) {
        const char* ic = keyword+1;
        while( *ic ) {
            if( !next() ) {
                fail(tsprintf("bad keyword, expected %s but EOF found", keyword));
            }
            if( this->current != *ic ) {
                fail(tsprintf("bad keyword, expected %s", keyword));
            }
            ic++;
        }
        TINFRA_TRACE(json_lexer_tracer, "readed keyword " << keyword);
        next();
    }
};
json_lexer::json_lexer(tinfra::input_stream& s):
    self(new internal_data())
{
    self->input = &s;
    self->current = -1;
    self->finished = 0;
    self->buffer_start = self->buffer;
    self->buffer_end = self->buffer;
    
    self->detect_encoding();
    self->next();
}
json_lexer::~json_lexer()
{
}

bool json_lexer::fetch_next(json_token& tok)
{
    while (! self->finished ) {
        switch( self->current ) {
        case '{':
            tok.type = json_token::OBJECT_BEGIN;
            self->next();
            TINFRA_TRACE(json_lexer_tracer, "readed OBJECT_BEGIN");
            return true;
        case '}':
            tok.type = json_token::OBJECT_END;
            self->next();
            TINFRA_TRACE(json_lexer_tracer, "readed OBJECT_END");
            return true;
        case '[':
            tok.type = json_token::ARRAY_BEGIN;
            self->next();
            TINFRA_TRACE(json_lexer_tracer, "readed OBJECT_BEGIN");
            return true;
        case ']':
            tok.type = json_token::ARRAY_END;
            self->next();
            return true;
        case ':': // 
            tok.type = json_token::COLON;
            self->next();
            return true;
        case ',': // COMMA
            tok.type = json_token::COMMA;
            self->next();
            return true;
        case '"':  // "string"
                   // NOTE, shall we support ' ??, RFC says that only " starts strings
            self->consume_string();
            tok.value = self->last_token;
            return true;
        case '-':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9': // numbers: integer or double
            tok.type  = self->consume_number();
            tok.value = self->last_token;
            return true;
        case 't': // keywords true
            self->consume_keyword("true");
            tok.type = json_token::TRUE;
            return true;
        case 'f': // keywords: false
            self->consume_keyword("false");
            tok.type = json_token::FALSE;
            return true;
        case 'n': // keywords: null
            self->consume_keyword("null");
            tok.type = json_token::TOK_NULL;
            return true;
        case ' ': case '\t': case '\r': case '\n': // whitespace
            self->next();
            continue;
        }
    }
    return false;
}

} // end namespace tinfra
