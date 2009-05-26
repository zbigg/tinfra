#include "tinfra/regexp.h"
#include "tinfra/tstring.h"
#include "tinfra/cmd.h"
#include "tinfra/fmt.h"

#include <tinfra/subprocess.h>
#include <tinfra/trace.h>

#include <string>
#include <iostream>

#include "tinfra/aio.h"
#include "tinfra/buffered_aio_adapter.h"

using tinfra::regexp;
using tinfra::tstring;
using tinfra::fmt;

/*
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
*/

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

class const_replacer {
	tstring replacement;
public:
	const_replacer(tstring const& r): replacement(r) {}
	
	std::wstring operator()(tstring const& i) {
		//std::cerr << tinfra::fmt("replacer(%s)->%s") % i % replacement << std::endl;
		return replacement.str();
	}
};

std::string clean_stl(tstring const& text)
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
        std::string result = text.str();
	const rule* i = rules;
	while( i->text != 0 ) {
		result = replacestr(i->text, result, const_replacer(i->replacement));
		++i;
	}
	return result;
}

struct string_sink {
    virtual void process(tstring const& line) = 0;
    
    virtual ~string_sink() {}
};

class colorizer_line_sink: public string_sink {
    const char*         prefix_;
    tinfra::io::stream* out_;
public:
    colorizer_line_sink(const char* prefix, tinfra::io::stream* out): prefix_(prefix), out_(out) {}

    virtual void process(tstring const& line) {
        std::string s = std::string(prefix_) + ": " + clean_stl(line);
        std::cerr.write(s.data(), s.size());
    }
};

class line_protocol: public tinfra::parser {
    string_sink& sink;
public:
    
    line_protocol(string_sink& s): sink(s),finished(false) {}
    virtual int   process_input(tinfra::tstring const& input)
    {
        return consume(input);
    }
    
    virtual void  eof(tinfra::tstring const& unparsed_input)
    {
        TINFRA_TRACE_MSG("eof signaled");
        consume(unparsed_input);
        finished = true;
    }
    
    bool finished;
    int consume(tstring current) {
        size_t consumed = 0;
        TINFRA_TRACE_VAR(current.size());
        while( current.size() > 0 ) {
            size_t eol_pos =  current.find_first_of('\n');
            if( eol_pos == tinfra::tstring::npos )
                break;
        
            consumed += eol_pos+1;
            sink.process(current.substr(0,eol_pos+1));
            current = current.substr(eol_pos+1);
        }
        TINFRA_TRACE_VAR(consumed);
        return consumed;
    }

    virtual ~line_protocol() {}
};

using std::auto_ptr;
using tinfra::aio::dispatcher;
using tinfra::io::stream;

class aio_colorizer: public tinfra::aio::listener {
    tinfra::io::stream* _out;
    colorizer_line_sink sink;
    line_protocol       protocol;
    tinfra::aio::buffered_aio_adapter aio_adapter;
    
public:
    aio_colorizer(const char* prefix, tinfra::io::stream* out) : 
        _out(out),
        sink(prefix, out),
        protocol(sink),
        aio_adapter(protocol),
        finished(false)
    {}
    
    bool finished;
    virtual void event(dispatcher& d, stream* c, int event)
    {
        aio_adapter.event(d,c,event);
    }
    
    virtual void failure(dispatcher& d, stream* c, int error)
    {
        aio_adapter.failure(d,c,error);
    }
    virtual void removed(dispatcher& d, stream* c)
    {
        finished = true;
    }
};
using tinfra::subprocess;
void process(tinfra::subprocess& sp)
{
    tinfra::io::stream* my_stdout = 0;
    tinfra::io::stream* my_stderr = 0;
    
    aio_colorizer stdout_colorizer("OUT", my_stdout);
    aio_colorizer stderr_colorizer("ERR", my_stderr);
    
    auto_ptr<dispatcher> D = dispatcher::create();
    D->add( sp.get_stdout(), &stdout_colorizer, dispatcher::READ);
    D->add( sp.get_stderr(), &stderr_colorizer, dispatcher::READ);
    
    while( !stdout_colorizer.finished || !stderr_colorizer.finished ) {
            D->step();
    
            tinfra::test_interrupt();
    }
}

int colorizer_main(int argc, char** argv)
{    
    if( argc == 1 )
        tinfra::cmd::fail("command needed");
    
    using std::vector;
    using std::string;
    vector<string> args(argv+1, argv+argc);
    
    auto_ptr<subprocess> sp = subprocess::create();
    
    sp->set_stdout_mode(subprocess::REDIRECT);
    sp->set_stderr_mode(subprocess::REDIRECT);    
    sp->start(args);
    process(*sp);
    sp->wait();
    int exit_code = sp->get_exit_code();
    return exit_code;
}

TINFRA_MAIN(colorizer_main);

