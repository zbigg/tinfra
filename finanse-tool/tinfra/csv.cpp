//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "csv.h"

#include "tinfra/string.h"
#include "tinfra/trace.h"

#include <cassert>

namespace tinfra {

namespace {    
const tstring LINE_DELIMITER = "\n";
}

TINFRA_MODULE_TRACER(tinfra_csv);
TINFRA_USE_TRACER(tinfra_csv);

csv_parser::csv_parser(char separator):
    in_quotes(false),
    SEPARATOR_CHAR(separator),
    builder_(memory_pool_),
    has_result_(false)
{
}
//
// IO adaptor callbacks (parser interface)
//
int   csv_parser::process_input(tinfra::tstring const& input)
{
    const tstring::size_type eol = input.find_first_of(LINE_DELIMITER);
    if( eol == tstring::npos )
        return 0;
    const tstring line = input.substr(0, eol+1);
    process_line(line);
    return line.size();
}

void  csv_parser::eof(tinfra::tstring const& unparsed_input)
{
    TINFRA_TRACE_VAR(unparsed_input);
    process_line(unparsed_input);
}

bool  csv_parser::get_result(csv_raw_entry& r) 
{
    if( has_result_ ) {
        r = entry_;
        has_result_ = false;
        return true;
    }
    return false;
}

//
// implementation details
//
namespace {
    typedef tstring::size_type pos_type;
    const tstring              EMPTY_STRING = "";
    const pos_type             NPOS         = tstring::npos;
    const char                 QUOTE_CHAR   = '"';
}

tstring csv_parser::process(tstring input, bool first)
{
    if( in_quotes ) {
        return process_quoted_entry(input);
    }
    
    if( first ) {
        return process_entry(input);
    }
    
    if( input.size() == 0 ) {
        return input;
    }
    
    if( input[0] == SEPARATOR_CHAR ) {
        return process_entry(input.substr(1));
    }
    assert(false);
}

tstring csv_parser::process_entry(tstring input)
{
    if( input.size() == 0 ) {
        entry_.push_back(EMPTY_STRING);
        return input;
    }
    if( input[0] == QUOTE_CHAR ) {
        return process_quoted_entry(input.substr(1));
    }
    const char DELIMITERS[] = { '\r', '\n', SEPARATOR_CHAR };
    pos_type delim_pos = input.find_first_of(DELIMITERS, 0, 3);
    if( delim_pos == NPOS ) {
        entry_.push_back(memory_pool_.alloc(input));
        return EMPTY_STRING;
    } else {
        const tstring ps = input.substr(0, delim_pos);
        entry_.push_back(memory_pool_.alloc(ps));
        if( input[delim_pos] == SEPARATOR_CHAR ) {
            return input.substr(delim_pos);
        } else {
            return EMPTY_STRING;
        }
    }
    
}

tstring csv_parser::process_quoted_entry(tstring input)
{
    while( true ) {
        const pos_type quote_pos = input.find_first_of(QUOTE_CHAR);
        if( quote_pos == NPOS ) {
            builder_.append(input);
            in_quotes = true;
            return EMPTY_STRING;
        }
        if( quote_pos < input.size()-1 && input[quote_pos+1] == QUOTE_CHAR ) {
            tstring ps = input.substr(0, quote_pos + 1);
            builder_.append(ps);
            input = input.substr(quote_pos+2);
            // g++ doesn't do TailCallElimination here. why ?
            //return process_quoted_entry(input.substr(quote_pos+2));
        }
        const tstring ps = input.substr(0, quote_pos);
        builder_.append(ps);
        const tstring ss = builder_.str();
        TINFRA_TRACE_VAR(ss);
        entry_.push_back(ss);
        builder_.reset();
        
        in_quotes = false;
        return input.substr(quote_pos+1);
    }
}

void csv_parser::process_line(tstring const& line)
{
    if( !has_result_ && !in_quotes )
        entry_.clear();
    tstring input = line;
    
    do {
        bool first = ! in_quotes && entry_.size() == 0;
        input = process(input, first);
        first = false;
    } while( input.size() > 0 );
    has_result_ = entry_.size() > 0;
}


raw_csv_reader::raw_csv_reader(std::istream& source, char sep) : 
    parser_(sep), 
    parser_adaptor_(source, parser_)
{
}

bool raw_csv_reader::fetch_next(csv_raw_entry& result)
{
    return parser_adaptor_.fetch_next(result);
}

std::string escape_csv(tstring const& value)
{
    std::ostringstream vvv;
    escape_csv(value, vvv);
    return vvv.str();
}

void escape_csv(tstring const& value, std::ostream& out)
{
    bool need_quote = false;
    tstring::size_type quote_pos = value.find_first_of('"');
    if( quote_pos != tstring::npos ) {
        need_quote = true;
    } else if( value.find_first_of(",\r\n") != tstring::npos ) {
        need_quote = true;
    }    
    if( !need_quote ) {
        out << value;
        return;
    }
    if( quote_pos == tstring::npos ) {
        out << '"' << value << '"';
        return;
    } else {
        out << '"';
        tstring input = value;
        TINFRA_TRACE_VAR(input);
        TINFRA_TRACE_VAR(quote_pos);
        do {
            size_t len = std::min(input.size(), quote_pos);
            out.write(input.data(), len);            
            if( quote_pos == tstring::npos )
                break;
            out << "\"\"";
            input = input.substr(quote_pos+1);
            quote_pos = input.find_first_of('"');
            TINFRA_TRACE_VAR(input);
            TINFRA_TRACE_VAR(quote_pos);
        } while( true );
        out << '"';
    }
}

} // end namespace tinfra

#ifdef BUILD_UNITTEST

#include <unittest++/UnitTest++.h>
#include <vector>

SUITE(tinfra_csv) {
    using tinfra::tstring;
    using tinfra::raw_csv_reader;
    using tinfra::csv_raw_entry;
    
    typedef std::vector<std::string> entry_type;
    entry_type one_line_parse(tstring const& csv)
    {
        std::istringstream s(csv.str());
        raw_csv_reader rrr(s);
        CHECK(rrr.has_next());
        csv_raw_entry& entry = rrr.next();
        
        entry_type result;
        
        for( csv_raw_entry::const_iterator i = entry.begin(); i != entry.end(); ++i ) {
            result.push_back(i->str());
        }
        
        CHECK(!rrr.has_next());
        return result;
    }
    
    TEST(csv_empty) {
        entry_type t = one_line_parse("");
        CHECK_EQUAL( 1, t.size());
        CHECK_EQUAL( "", t[0]);
    }
    TEST(csv_one1) {
        entry_type t = one_line_parse("a");
        CHECK_EQUAL( 1, t.size());
        CHECK_EQUAL( "a", t[0]);
    }
    
    TEST(csv_basic) {
        entry_type t = one_line_parse("abc,def,geh");
        CHECK_EQUAL( 3, t.size());
        CHECK_EQUAL( "abc", t[0]);
        CHECK_EQUAL( "def", t[1]);
        CHECK_EQUAL( "geh", t[2]);
    }

    TEST(csv_empties) {
        entry_type t = one_line_parse(",,,");
        CHECK_EQUAL( 4, t.size());
        CHECK_EQUAL( "", t[0]);
        CHECK_EQUAL( "", t[1]);
        CHECK_EQUAL( "", t[2]);
        CHECK_EQUAL( "", t[3]);
    }
    
    TEST(csv_quotes) {
        entry_type t = one_line_parse("\"\",\"def\",geh,\"a\"");
        CHECK_EQUAL( 4, t.size());
        CHECK_EQUAL( "",    t[0]);
        CHECK_EQUAL( "def", t[1]);
        CHECK_EQUAL( "geh", t[2]);
        CHECK_EQUAL( "a",   t[3]);
    }
    
    TEST(csv_dbl_quotes) {
        entry_type t = one_line_parse("\"\"\"\"");
        CHECK_EQUAL( 1, t.size());
        CHECK_EQUAL( "\"",    t[0]);
    }
    
    TEST(csv_multiline) {
        entry_type t = one_line_parse("\"abc\r\ndef\",xyz");
        CHECK_EQUAL( 2, t.size());
        CHECK_EQUAL( "abc\r\ndef",    t[0]);
        CHECK_EQUAL( "xyz",    t[1]);
    }
    
    
    TEST(escape_csv) {
        using tinfra::escape_csv;
        CHECK_EQUAL( "",             escape_csv("") );
        CHECK_EQUAL( "a",            escape_csv("a") );
        CHECK_EQUAL( "\"\r\"",       escape_csv("\r") );
        CHECK_EQUAL( "\"\r\n\"",     escape_csv("\r\n") );
        CHECK_EQUAL( "\"\n\"",       escape_csv("\n") );
        CHECK_EQUAL( "\"\"\"\"",     escape_csv("\"") );
        CHECK_EQUAL( "\"a\"\"b\"",   escape_csv("a\"b") );
    }
}

#endif // BUILD_UNITTEST

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++

