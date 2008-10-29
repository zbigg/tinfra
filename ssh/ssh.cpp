#include "tinfra/tinfra.h"
#include "tinfra/symbol.h"

#include "tinfra/io/stream.h"

#include <sstream>
#include <stdexcept>

// TODO: move to specific utility module
class field_printer {
    std::ostream& out;
    bool need_separator;
public:
    field_printer(std::ostream& o) : out(o), need_separator(false) {}
    
    template <class T>
    void operator () (const tinfra::symbol& s, T const& t) 
    {
        if( need_separator ) {
            out << ", ";
        }
        out << s.str() << " = " << t;
        need_separator = true;
    }
    
    template <typename T>
    void managed_struct(T const& v, tinfra::symbol const& s)
    {
        if( need_separator ) 
            out << ", ";
        out << tinfra::TypeTraitsGeneric<T>::name() << " { ";
        need_separator = false;
        tinfra::tt_process<T>(v, *this);
        out << " }";
        need_separator = true;
    }
};

template<typename T>
std::string struct_to_string(T const& v)
{
    std::ostringstream s;
    field_printer printer(s);
    
    s << tinfra::TypeTraitsGeneric<T>::name() << " { ";
    tinfra::tt_process(v, printer);
    s << "}";
    return s.str();
}

std::string hexify(const char* buf, size_t length)
{
    std::ostringstream r;
    for(int i = 0; i < length; ++i ) {
        if( i > 0 )
            r << " ";
        r << std::setw(2) << std::setfill('0') << std::hex << (int)(unsigned char)(buf[i]);
    }
    return r.str();
}

using std::auto_ptr;
using tinfra::io::stream;

#define DOUT(a) do { std::cerr << __FILE__ << ":" << __LINE__ << ": " << a << std::endl; } while(false)
#define INFORM(a,b) do { std::cerr << a << ": " <<  b << std::endl; } while(false)

namespace utils {

void read_for_sure(stream* s, char* buffer, size_t size)
{
    size_t readed = 0;
    while( readed < size ) {
        int r = s->read(buffer + readed, size - readed);
        if( r <= 0 ) {
            throw std::runtime_error("unexpected EOF");
        }
        readed += r;
    }
}

void read_until(stream* s, std::string& result, std::string const& delim)
{
	while( true ) {
		char c;
		int r = s->read(&c, sizeof(char));
		if( r == 0 ) {
			throw std::runtime_error("EOF");
		}
		result.append(1, c);
		if( result.length() > 0 ) 
	}
}

}

namespace ssh {
	using tinfra::io::stream;
	
	void perform_invitation(stream* stream);
}

namespace ssh {

void perform_invitation(stream* stream)
{
	const std::string line_delimiter = "\r\n";
	const std::string expected_begining = "SSH-2.0-";
	while( true ) {
		std::string line;
		read_until(stream, line, line_delimiter);
		if( line.size() >= expected_begining &&
		    line.find(expected_begining) == 0) 
		{
			INFORM("version string", line);
			break;
		} else {
			INFORM("user info", line);
		}
	    
	}
}

}


int ssh_main(int argc, char** argv)
{
	auto_ptr<stream> connection = tinfra::io::socket::open_client_socket("localhost",22);
	
	ssh::pefrorm_invitation()
	return 0;
}
TINFRA_MAIN(ssh_main);
