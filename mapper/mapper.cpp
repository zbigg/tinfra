//#include <funtional>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <vector>
#include <string>

struct A {
	int id;
	std::vector<std::string> names;
};

struct json_renderer {
	std::ostream& out;
	bool need_separator;

	json_renderer(std::ostream& out):
		out(out),
		need_separator(false)
	{
	}
	void begin_map() { maybe_separate(); out << "{"; }
	void end_map()   { out << '}'; need_separator = true; }
	void begin_seq() { maybe_separate(); out << "["; }
	void end_seq()   { out << "]"; need_separator = true; }

	template <typename T>
	void value(T const& v) {
		maybe_separate();
		out << v;
		need_separator = true;
	}

	void value(std::string const& v) {
		maybe_separate();
		out << "'" << v << "'";
		need_separator = true;
	}

	void maybe_separate() {
		if( need_separator )
			comma();
	}
	void separate(char c) {
		out << c << ' ';
		need_separator = false;
	}
	void comma() {
		separate(','); 
	}

	template <typename K, typename V>
	void nvm_pair(K const& k, V const& v) {
		value(k);
		separate(':');
		value(v);

		need_separator = true;
	}
};
void render_json(A const& a, json_renderer& json)
{
	json.begin_map();
	json.nvm_pair("id", a.id);
	json.begin_seq();
	for( int i = 0; i < a.names.size(); ++i ) {
	    json.value(a.names[i]);
	}
	json.end_seq();
	json.end_map();
}

struct json_output {
	template <typename T>
	static void render(T const& v, std::ostream& out)
	{
		out << "Content-type: text/json\r\n";
		out << "\r\n";
		json_renderer foo(out);
		render_json(v, foo);
		out << "\n";
	}
};

struct request {
};
struct response {
};

struct handler {
	virtual void handle_request(request&, response& resp) = 0;
};

template <typename Renderer, typename R>
class foo_handler: public handler {
	boost::function< R (request&) > fun;
public:
	foo_handler(boost::function<R (request&) > f): fun(f) {}

	void handle_request(request& req, response& resp)
	{
		R result = fun(req);
		// TBD: resp
		Renderer::render(result, std::cout);
	}
};

template <typename R, typename T>
R one_arg(boost::function<R(T)> fun, request& r)
{
	const T v1 = 777;
	const R result = fun(v1);
	return result;
}

template <typename R, typename T1, typename T2>
R two_arg(boost::function<R(T1, T2)> fun, request& r)
{
	const T1 v1 = 999;
	const T2 v2 = 888;
	const R result = fun(v1,v2);
	return result;
}
struct map_entry {
	std::string pattern;
	handler* h;
};

std::vector<map_entry> mapping;

template <typename Renderer, typename R, typename T>
void add_function(std::string url, boost::function<R(T)> fun)
{
	map_entry e;
	e.pattern = url;
	e.h = new foo_handler<Renderer, R>(boost::bind(&one_arg<R,T>, fun, _1));
	mapping.push_back(e);
}

template <typename Renderer, typename R, typename T1, typename T2>
void add_function(std::string url, boost::function<R(T1,T2)> fun)
{
	map_entry e;
	e.pattern = url;
	e.h = new foo_handler<Renderer, R>(boost::bind(&two_arg<R,T1,T2>, fun, _1));
	mapping.push_back(e);
}



void call(int idx, std::string url)
{
	handler* h = mapping[idx].h;

	request req;
	response resp;

	h->handle_request(req, resp);
}

A get_a(int id)
{
	A a;
	a.id = id;
	a.names.push_back("fake");
	return a;
}

A get_b(int id, int const& x)
{
	A a;
	a.id = x;
	a.names.push_back("fake2");
	return a;
}

int main()
{
	json_renderer json(std::cout);
	A a;
	a.id = 677;
	a.names.push_back("dupa");
	a.names.push_back("abc");
	render_json(a, json);
	std::cout << '\n';

	add_function<json_output>("/get_a/*/*.json", boost::function<A(int)>(&get_a));
	add_function<json_output,A,int,int>("/get_b/*/*.json", & get_b);

	call(0, "foo");
	call(1, "bar");
}

