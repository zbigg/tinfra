//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_inifile_h_included
#define tinfra_inifile_h_included

#include "tinfra/generator.h"
#include "tinfra/stream.h"

#include <memory> // for std::auto_ptr
#include <string> // for std::string

namespace tinfra {
namespace inifile {

enum entry_type {
    EMPTY,
    COMMENT,
    SECTION,
    ENTRY,
    INVALID
};

struct entry {
    entry_type  type;
    std::string name;
    std::string value;
};


class parser: public generator_impl<parser, entry> {
public:
    parser(tinfra::input_stream& in);
    ~parser();

    bool fetch_next(entry&);
    
private:
    struct internal_data;
    std::auto_ptr<internal_data> data_;
};

struct full_entry {
    std::string section;
    std::string name;
    std::string value;
};

class reader: public generator_impl<reader, full_entry> {
public:
	reader(tinfra::input_stream& in);
	~reader();
	
	bool fetch_next(full_entry&);
private:
	parser      p;
	std::string section;
	int line;
};

class writer {
    tinfra::output_stream& out;
public:
    writer(tinfra::output_stream& out);
    ~writer();
    
    void section(std::string const& name);
    void entry(std::string const& name, std::string const& value);
    void comment(std::string const& content);
    
    void entry(tinfra::inifile::entry const& e);
};

} // end namespace tinfra::inifile

typedef inifile::parser inifile_parser;
typedef inifile::entry  inifile_entry;

} // end namespace tinfra

#endif // tinfra_inifile_h_included
