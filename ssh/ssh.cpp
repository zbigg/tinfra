#include "tinfra/typeinfo.h" // for tinfra::type_name
//#include "tinfra/symbol.h"


#include <botan/botan.h>

#include <tinfra/tcp_socket.h>
#include <tinfra/stream.h>

#include "tinfra/string.h"
#include "tinfra/fmt.h"

#include <memory>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <iostream>


std::string hexify(const char* buf, size_t length)
{
    std::ostringstream r;
    for(size_t i = 0; i < length; ++i ) {
        if( i > 0 )
            r << " ";
        r << std::setw(2) << std::setfill('0') << std::hex << (int)(unsigned char)(buf[i]);
    }
    return r.str();
}

using std::auto_ptr;
using tinfra::input_stream;

#define DOUT(a) do { std::cerr << __FILE__ << ":" << __LINE__ << ": " << a << std::endl; } while(false)
#define INFORM(a,b) do { std::cerr << a << ": " <<  b << std::endl; } while(false)

namespace utils {

void read_for_sure(input_stream* s, char* buffer, size_t size)
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

void read_until(input_stream* s, std::string& result, std::string const& delim, size_t max_length=32768)
{
	while( true ) {
		char c;
		int r = s->read(&c, sizeof(char));
		if( r == 0 ) {
			throw std::runtime_error("EOF");
		}
		result.append(1, c);
		if( result.length() >= delim.size() ) {
			if( result.find(delim) == result.size()-delim.size() ) {
				return;
			}
		}
		if( result.length() >= max_length ) {
			throw std::runtime_error("too long input line");
		}
	}
}

}

namespace ssh {	
	void perform_invitation(tinfra::input_stream* stream);
}

namespace ssh {

enum protocol_version {
	V199,
	V200
};

struct protocol_state {
	protocol_version version;
};

void perform_invitation(tinfra::input_stream* stream, protocol_state& ps)
{
	const std::string line_delimiter = "\n";
	const std::string expected_begining = "SSH-";	
	while( true ) {
		std::string line;		
		utils::read_until(stream, line, line_delimiter);
		tinfra::strip_inplace(line);
		if( line.size() >= expected_begining.size() &&
		    line.find(expected_begining) == 0) 
		{
			INFORM("version string", line);			
			if( line.compare(expected_begining.length(), 4, "1.99") == 0 ) {
				ps.version = V199;
			} else if( line.compare(expected_begining.length(), 3, "2.0") == 0 ) {
				ps.version = V200;
			} else {
				throw std::runtime_error(tinfra::fmt("unsupported protocol version '%s'") % line);
			}
			break;
		} else {
			INFORM("user info", line);
		}
	}
}

using tinfra::rfc4251::uint32;
using tinfra::rfc4251::byte;

using tinfra::tstring;

struct binary_packet {
	uint32	packet_length;
	byte    padding_length;
	tstring payload;
	tstring padding;
	tstring mac;
};

}

#include "tinfra/cmd.h"

int ssh_main(int argc, char** argv)
{
	Botan::LibraryInitializer botan_lib;
        assert(argc>1);
	tinfra::tcp_client_socket socket(argv[1],22);
	ssh::protocol_state ps;
	ssh::perform_invitation(&socket, ps);
	return 0;
}

TINFRA_MAIN(ssh_main);
