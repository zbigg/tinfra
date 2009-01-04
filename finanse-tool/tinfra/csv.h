//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_csv_h__
#define __tinfra_csv_h__

#include <vector>
#include <string>
#include <istream>

#include "tinfra/tstring.h"

#include "tinfra/sequence_parser.h"
#include "tinfra/istream_sequence_parser.h"

namespace tinfra {

typedef std::vector<tstring> csv_raw_entry;
namespace foobar {
    // TODO: fake string_builder to be reeingeineered and moved to tinfra
    class string_builder {
        string_pool&    pool_;		
        std::string     buf_;
            
    public:
        string_builder(string_pool& p): pool_(p) {}
        
        void append(tstring const& s) {
            buf_.append(s.data(), s.size());
        }
        
        
        void reset() {
            buf_.clear();
        }
        
        tstring str() {
            return pool_.alloc(buf_);
        }
    };
}
class csv_parser: public sequence_parser<csv_raw_entry> {
public:
    csv_parser(char separator = ',');
    virtual ~csv_parser() {}
    //
    // IO adaptor callbacks (parser interface)
    //
    virtual int   process_input(tinfra::tstring const& input);
    
    virtual void  eof(tinfra::tstring const& unparsed_input);
    
    virtual bool  get_result(csv_raw_entry& r) ;
    
private:    
    void process_line(tstring const& line);
    
    bool           in_quotes;
    const char     SEPARATOR_CHAR;
    csv_raw_entry  entry_;
    string_pool    memory_pool_;
    foobar::string_builder builder_;
    bool           has_result_;
};

class raw_csv_reader: public generator_impl<raw_csv_reader, csv_raw_entry> {
public:
    raw_csv_reader(std::istream& byte_source, char separator = ',');
    
    bool fetch_next(csv_raw_entry&);
    
private:
    csv_parser parser_;
    istream_sequence_parser<csv_raw_entry> parser_adaptor_;    
};

std::string escape_csv(tstring const& value);
void escape_csv(tstring const& value, std::ostream& out);

} // end namespace tinfra

#endif // __tinfra_csv_h__

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++
