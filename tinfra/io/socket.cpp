#include "tinfra/io/stream.h"
#include "tinfra/io/socket.h"
#include "tinfra/fmt.h"
#include "tinfra/string.h" // debug only
#include <iostream> // debug only

#ifdef _WIN32
#include <winsock.h>

#define TS_WINSOCK

#else

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#define TS_BSD
#endif

using namespace std;

namespace tinfra {
namespace io {
namespace socket {

/*
class stream {
    virtual ~stream() {}
    enum {
        start,
        end,
        current
    } seek_origin;        
    void close() = 0;
    int seek(int pos, seek_origin origin = start) = 0;
    int read(char* dest, int size) = 0;
    int write(const char* data, int size) = 0;
};
*/

#ifdef TS_WINSOCK
typedef SOCKET socket_type;
#else
typedef int    socket_type;
#endif

static const socket_type invalid_socket = static_cast<socket_type>(-1);

static void close_socket(socket_type socket);
static void throw_socket_error(const char* message);

// TODO this log facility should be tinfra-wide
#ifdef _DEBUG
static void L(const std::string& msg)
{
    //std::cerr << "socket: " << escape_c(msg) << std::endl;
}
#else
#define L(a) (void)0
#endif

class socketstream: public stream {
    socket_type socket_;
public:
    socketstream(socket_type socket): socket_(socket) {}
    ~socketstream() {
        if( socket_ != invalid_socket )
            close();
    }
    void close() {        
        close_socket(socket_);
        socket_ = invalid_socket;        
    }
    int seek(int pos, stream::seek_origin origin)
    {
        throw io_exception("sockets don't support seek()");
    }
    int read(char* data, int size)
    {
        L(fmt("%i: reading ...") % socket_);
        int result = ::recv(socket_, data ,size, 0);
        L(fmt("%i: readed %i '%s'") % socket_ % result % std::string(data,result));
        if( result == -1 ) {
            throw_socket_error("unable to read from socket");
        }
        return result;
    }
    int write(const char* data, int size)
    {
        L(fmt("%i: send '%s'") % socket_ % std::string(data,size));
        int result = ::send(socket_, data, size, 0);
        L(fmt("%i: sent %i") % socket_ % result);
        if( result == -1 ) {
            throw_socket_error("unable to write to socket");
        }
        return result;
    }
    void sync() 
    {
    }
    socket_type get_socket() const { return socket_; }
};

#ifdef _WIN32

std::string win32_strerror(DWORD error_code)
{
    LPVOID lpMsgBuf;
    if( ::FormatMessage(
	FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
	NULL,
	error_code,
	MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
	(LPTSTR) &lpMsgBuf,
	0,
	NULL
	) < 0 || lpMsgBuf == NULL) {

	return fmt("unknown error: %i") % error_code;
    }
    std::string result((char*)lpMsgBuf);
    ::LocalFree(lpMsgBuf);
    return result;
}

#endif

static void ensure_socket_initialized()
{
#ifdef TS_WINSOCK
    static bool winsock_initialized = false;
    if( winsock_initialized ) return;
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    wVersionRequested = MAKEWORD(1, 1);

    err = WSAStartup(wVersionRequested, &wsaData);

    if (err != 0)
	throw io_exception(fmt("unable to initialize WinSock system: %s") % win32_strerror(err));

    if ( LOBYTE( wsaData.wVersion ) != 1 ||
	   HIBYTE( wsaData.wVersion ) != 1 ) {
        WSACleanup();
        errno = ENODEV;
        throw io_exception("bad winsock version");
    }
    winsock_initialized = true;
#endif
}

static void throw_socket_error(const char* message)
{
#ifdef TS_WINSOCK
    int error_code = WSAGetLastError();
    throw io_exception(fmt("%s: %s") % message % win32_strerror(error_code));
#else
    throw io_exception(fmt("%s: %s") % message % strerror(errno));
#endif
}
static void close_socket(socket_type socket)
{
#ifdef TS_WINSOCK
    int rc = ::closesocket(socket);
#else
    int rc = ::close(socket);
#endif
    if( rc == -1 ) throw_socket_error("unable to close socket");
}

static socket_type create_socket()
{
    ensure_socket_initialized();
    socket_type result = ::socket(AF_INET,SOCK_STREAM,0);
    if( result == invalid_socket ) 
        throw_socket_error("socket creation failed");
    return result;
}
#ifndef INADDR_NONE
#define INADDR_NONE -1
#endif

static void get_inet_address(const char* address,int rport, struct sockaddr_in* sa)
{
    ensure_socket_initialized();
    
    std::memset(sa,0,sizeof(*sa));
    sa->sin_family = AF_INET;
    sa->sin_port = htons((short)rport);

    ::in_addr     ia;
    unsigned long ian =  ::inet_addr(address);
    ia.s_addr = ian;
    if( ian == INADDR_NONE ) {
        ::hostent*    ha;
        ha = ::gethostbyname(address);
        if( ha == NULL )
	    throw io_exception(fmt("unable to resolve: %s") % address);
    	std::memcpy(&sa->sin_addr, ha->h_addr, ha->h_length);
    } else {
        /* found with inet_addr or inet_aton */
        std::memcpy(&sa->sin_addr,&ia,sizeof(ia));
    }
}

stream* open_client_socket(char const* address, int port)
{    
    ::sockaddr_in sock_addr;
    
    get_inet_address(address, port, &sock_addr);
    
    socket_type s = create_socket();
    
    if( ::connect(s, (struct sockaddr*)&sock_addr,sizeof(sock_addr)) < 0 ) {
        close_socket(s);
        throw_socket_error(fmt("unable to connect %s:%i") % address % port);
    }
    return new socketstream(s);
}

stream* open_server_socket(char const* listening_host, int port)
{
    ::sockaddr_in sock_addr;
    std::memset(&sock_addr,0,sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    if( listening_host ) {
        get_inet_address(listening_host, port, &sock_addr);
    } else {
        sock_addr.sin_port = htons((short) port);
    }
    
    socket_type s = create_socket();
    
    if( ::bind(s,(struct sockaddr*)&sock_addr,sizeof(sock_addr)) < 0 ) {
        close_socket(s);
        throw_socket_error("bind failed");
    }

    if( ::listen(s,5) < 0 ) {
        close_socket(s);
        throw_socket_error("listen failed"); 
    }
    
    return new socketstream(s);
}

stream* accept_client_connection(stream* server_socket)
{
    socketstream* socket = dynamic_cast<socketstream*>(server_socket);
    if( !socket ) throw_socket_error("accept: not a socketstream");
    
    socket_type accept_sock = ::accept(socket->get_socket(), NULL, NULL );
    if( accept_sock == invalid_socket ) throw_socket_error("accept failed");
        
    return new socketstream(accept_sock);
}

} } } //end namespace tinfra::io::socket

