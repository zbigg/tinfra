#include "tinfra/regexp.h"
#include "tinfra/tstring.h"
#include "tinfra/cmd.h"
#include "tinfra/fmt.h"

#include <string>
#include <iostream>

#include <cxxabi.h>

struct whole_match_result: public tinfra::match_result_processor {
    void prepare(int groups) { }
    void match_group(int group_no, const char* str, size_t pos, size_t len) {
	if( group_no != 0 ) return;
        this->pos = pos;
	this->len = len;
    }
    
    size_t pos;
    size_t len;
};

using tinfra::regexp;
using tinfra::tstring;
using tinfra::fmt;

template <typename FUN>
std::string replace(regexp const& re, tstring const& input, FUN functor)
{
	whole_match_result match;
	size_t pos = 0;
	std::string result;
	while( re.matches(input.substr(pos), match) ) {
		
		if( match.pos > 0 ) {
			tstring rest = input.substr(pos, match.pos); 
			result.append(rest.data(), rest.size());
		}			
		tstring matched = input.substr(pos + match.pos, match.len);
		//std::cerr << fmt("RE: match(%i,%i) -> '%s'\n") % match.pos % match.len % matched;
		result.append(functor(matched));
		pos = match.pos + match.len;
	}
	if( pos < input.size()) {
		tstring rest = input.substr(pos);
		result.append(rest.data(), rest.size());
	}
	return result;
}

template <typename FUN>
std::string replacestr(tstring const& hard_text, tstring const& input, FUN functor)
{
	size_t pos = 0;
	std::string result;
	size_t match_pos;
	const size_t match_len = hard_text.size(); 
	while( (match_pos = input.find(hard_text, pos)) != tstring::npos) {		
		if( match_pos > 0 ) {
			tstring rest = input.substr(pos, match_pos-pos); 
			result.append(rest.data(), rest.size());
		}			
		tstring matched = input.substr(match_pos, match_len);
		//std::cerr << fmt("RH: match(%i,%i) -> '%s'\n") % match_len % match_len % matched;
		result.append(functor(matched));
		pos = match_pos + match_len;
	}
	if( pos < input.size()) {
		tstring rest = input.substr(pos);
		result.append(rest.data(), rest.size());
	}
	return result;
}

std::string demangle_typeinfo_name(const char* symbolname)
{
    int status = 0;
    char* data = abi::__cxa_demangle(symbolname, NULL, NULL, &status);
    switch( status ) {
    case 0:
        {
            std::string result = data;
	    //std::cerr << tinfra::fmt("demangle_typeinfo_name(%s)->%s") % symbolname % result << std::endl;
            ::free(data);
            return result;
        }
    case -1:
    case -2:
    case -3:
    default:
        return symbolname;
    }
}


std::string demangler(tstring const& symbolname)
{
	if( symbolname[0] == '<')
		return demangler(symbolname.substr(1, symbolname.size()-2));
	size_t ape_pos = symbolname.find_first_of("@+");
	if( ape_pos != tstring::npos )
		return demangler(symbolname.substr(0,ape_pos));
	std::string buf(symbolname.str());
	return tinfra::fmt("<%s>") % demangle_typeinfo_name(buf.c_str());
}

class const_replacer {
	tstring replacement;
public:
	const_replacer(tstring const& r): replacement(r) {}
	
	std::string operator()(tstring const& i) {
		//std::cerr << tinfra::fmt("replacer(%s)->%s") % i % replacement << std::endl;
		return replacement.str();
	}
};

std::string clean_stl(std::string const& text)
{
	static const struct rule {
		const char* text;
		const char* replacement;
	} rules[] = {
		{ "std::basic_ostream<char, std::char_traits<char> >", "std::ostream" },
		{ "std::basic_string<char, std::char_traits<char>, std::allocator<char> >", "std::string" },
		{ "std::basic_ostringstream<char, std::char_traits<char>, std::allocator<char>", "std::ostringstream" },
		{ "std::basic_streambuf<char, std::char_traits<char> >", "std::streambuf" },
		{ 0,0 }
	};
	std::string result = text;
	const rule* i = rules;
	while( i->text != 0 ) {
		result = replacestr(i->text, result, const_replacer(i->replacement));
		++i;
	}
	return result;
}
int demangler_main(int argc, char** argv)
{
    std::string line;
    const tinfra::regexp RE("<_(.*)>");
    
    while( std::getline(std::cin, line) ) {
	    std::string demangled = replace(RE, line, &demangler);
	    std::string cleaned = clean_stl(demangled);
	    std::cout << cleaned << std::endl;
    }
    return 0;    
}

TINFRA_MAIN(demangler_main)
