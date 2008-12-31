#include "csv.h"

#include "tinfra/string.h"

namespace tinfra {

raw_csv_reader::raw_csv_reader(std::istream& source, char sep)
    : byte_source_(source), separator_(sep)
{
}

bool raw_csv_reader::fetch_next(csv_raw_entry& result)
{
    std::string current_line_;
    getline(byte_source_, current_line_);
    strip_inplace(current_line_);
    if( current_line_.size() == 0 ) {
        return false;
    }
    
    const char QUOTE_CHAR = '"';
    const char SEPARATOR_CHAR = separator_;
    
    const char DELIMITERS[2] = { SEPARATOR_CHAR, QUOTE_CHAR };
    const tstring::size_type NPOS = tstring::npos;
    typedef tstring::size_type pos_type;
    result.clear();
    
    pos_type current_pos = 0;
    bool in_quotes = false;
    
    memory_pool_.clear();
    
    std::string tmp;
    while( current_pos <= current_line_.size() ) {
        bool consume = false;
        if( ! in_quotes ) {
            pos_type entry_delim_pos = current_line_.find_first_of(DELIMITERS, current_pos, 2);
            if( entry_delim_pos == NPOS ) {
                entry_delim_pos = current_line_.size();
                consume = true;
            } else if( current_line_[entry_delim_pos] == QUOTE_CHAR ) {
                in_quotes = true;
                current_pos = entry_delim_pos+1;
                continue;
            } else if( current_line_[entry_delim_pos] == SEPARATOR_CHAR ) {
                consume = true;
            } else {
                assert(false);
            }
            if( consume ) {
                const size_t len = entry_delim_pos - current_pos;
                const tstring entry = memory_pool_.alloc( tstring(current_line_.data() + current_pos, len));
                result.push_back(entry);
                current_pos = entry_delim_pos+1;
            }      
        } else { // we're in quotes
            pos_type quote_pos = current_line_.find_first_of(QUOTE_CHAR, current_pos);
            if( quote_pos == NPOS ) {
                tmp.append( current_line_.data() + current_pos, NPOS);
                getline(byte_source_, current_line_);
                strip_inplace(current_line_);
                current_pos = 0;
                continue;
            } else {
                if( quote_pos < current_line_.size()-1 && current_line_[quote_pos+1] == QUOTE_CHAR ) {
                    {
                        const size_t len = quote_pos - current_pos;
                        tmp.append(current_line_.data() + current_pos, len);
                    }
                    tmp.append(1, '"');
                    current_pos = quote_pos+2;
                    continue;
                } else {
                    const size_t len = quote_pos - current_pos;
                    tmp.append(current_line_.data() + current_pos, len);
                    
                    result.push_back(memory_pool_.alloc(tstring(tmp)) );
                    current_pos = quote_pos+1;
                    in_quotes = false;
                    continue;
                }
            }
        }
          
    }
    return true;
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
}
#endif


// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++
