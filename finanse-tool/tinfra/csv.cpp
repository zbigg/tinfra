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
    lazy_byte_consumer<csv_parser>(*this),
    in_quotes(false),
    SEPARATOR_CHAR(separator),
    builder_(memory_pool_),
    has_result_(false)
{
    wait_for_delimiter(LINE_DELIMITER, make_step_method(&csv_parser::have_full_line));
    TINFRA_TRACE_VAR(SEPARATOR_CHAR);
}
//
// IO adaptor callbacks (parser interface)
//
int   csv_parser::process_input(tinfra::tstring const& input)
{
    std::string inputc = tinfra::escape_c(input);
    TINFRA_TRACE_VAR(inputc);
    int r =  process(input);
    TINFRA_TRACE_VAR(r);
    return r;
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
int csv_parser::have_full_line(tstring const& input)
{
    tstring::size_type eol = input.find_first_of(LINE_DELIMITER);
    std::string inputc = tinfra::escape_c(input);
    TINFRA_TRACE_VAR(inputc);
    TINFRA_TRACE_VAR(eol);
    assert(eol != tstring::npos); 
    tstring line = tstring(input.data(), eol+1);
    process_line(line);
    wait_for_delimiter(LINE_DELIMITER, make_step_method(&csv_parser::have_full_line));
    return line.size();
}

void csv_parser::process_line(tstring const& line)
{
    typedef tstring::size_type pos_type;
    const pos_type NPOS = tstring::npos;
    const char QUOTE_CHAR = '"';
    pos_type current_pos = 0;
    TINFRA_TRACE_VAR(line);
    if( !has_result_ )
        entry_.clear();
    while( current_pos < line.size() ) {
        if( in_quotes ) {
            const pos_type quote_pos = line.find_first_of(QUOTE_CHAR, current_pos);
            // no ending " until EOL
            if( quote_pos == NPOS ) {
                tstring ss = line.substr(current_pos);
                builder_.append(ss);
                return;
            }
            // the "" in quotes case
            if( quote_pos < line.size()-1 && line[quote_pos+1] == QUOTE_CHAR ) {
                tstring ps = line.substr(current_pos, quote_pos-current_pos + 1);
                TINFRA_TRACE_VAR(ps);
                builder_.append(ps);
                current_pos = quote_pos + 2;
                continue;
            }
            {
                const tstring ps = line.substr(current_pos, quote_pos-current_pos);
                TINFRA_TRACE_VAR(ps);
                builder_.append(ps);
            }
            const tstring ss = builder_.str();
            TINFRA_TRACE_VAR(ss);
            entry_.push_back(ss);
            
            current_pos = quote_pos+1;
            if( line[current_pos] == SEPARATOR_CHAR)
                current_pos++;
            in_quotes = false;
        } else if( line[current_pos] == QUOTE_CHAR ) {
            // the " at start of after SEPARATOR
            current_pos += 1;
            in_quotes = true;
            builder_.reset();
            continue;
        } else {
            // other case - find the end
            const char DELIMITERS[] = { '\r', '\n', SEPARATOR_CHAR };
            pos_type delim_pos = line.find_first_of(DELIMITERS, current_pos, 3);
            TINFRA_TRACE_VAR(delim_pos);
            if( delim_pos == NPOS ) {
                delim_pos = line.size();
            }
            TINFRA_TRACE_VAR(current_pos);
            if( delim_pos != 0 ) {
                const tstring ss = memory_pool_.alloc( line.substr(current_pos, delim_pos - current_pos) );
                TINFRA_TRACE_VAR(ss);
                entry_.push_back(ss);
            }
            
            current_pos = delim_pos+1;
        }
    }
    if( entry_.size() ) 
        has_result_ = true;
}


raw_csv_reader::raw_csv_reader(std::istream& source, char sep) : 
    parser_(sep), 
    parser_adaptor_(* source.rdbuf(), parser_)
{
}

bool raw_csv_reader::fetch_next(csv_raw_entry& result)
{
    return parser_adaptor_.fetch_next(result);
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
        std::istringstream s("");
        raw_csv_reader rrr(s);
        CHECK(! rrr.has_next());
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
    
}

#endif // BUILD_UNITTEST

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++

