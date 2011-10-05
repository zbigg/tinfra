//
// Copyright (c) 2011, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_text_h_included
#define tinfra_text_h_included

// tinfra deps
#include "generator.h"
#include "stream.h"

// other deps
#include <string>

namespace tinfra {

/*
class basic_tokenizer: public generator_impl<reader, std::string> {
public:
	basic_tokenizer(tinfra::input_stream& in, std::string const& delimiters, int flags);
	~basic_tokenizer();
	
	bool fetch_next(std::string&);
private:
	tinfra::input_stream& in;
	std::string delimiters;
	
};
*/

class line_reader: public generator_impl<line_reader, std::string> {
public:
	line_reader(tinfra::input_stream& in);
	~line_reader();
	
	bool fetch_next(std::string&);
private:
	// noncopyable
    line_reader(line_reader const&);
    line_reader& operator=(line_reader const&);

	tinfra::input_stream& in;
};

}

#endif
