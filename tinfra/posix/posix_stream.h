//
// Copyright (c) 2010-2011, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_posix_stream_h_included
#define tinfra_posix_stream_h_included

#include "tinfra/stream.h"

namespace tinfra {
namespace posix {

class file_descriptor {
public:
	typedef int handle_type;
	explicit file_descriptor(handle_type existing, bool own = true);
	~file_descriptor();
	
	void reset(handle_type fd, bool own);
	void close();
	
	handle_type handle() const { return this->fd; }
private:
	// non-copyable
	file_descriptor(const file_descriptor&);
	file_descriptor& operator=(const file_descriptor&);
	
	int fd;
	bool own;
};

class native_input_stream: public tinfra::input_stream {
public:
	explicit native_input_stream(file_descriptor::handle_type existing, bool own = true);
	~native_input_stream();
	
	// input_stream implementation
	void close();
	int read(char* dest, int size);
private:
	file_descriptor fd;
};

class native_output_stream: public tinfra::output_stream {
public:
	explicit native_output_stream(file_descriptor::handle_type existing, bool own = true);
	~native_output_stream();
	
	// output_stream implementation
	using tinfra::output_stream::write;
	void close();
	int write(const char* data, int size);
    	void sync();
private:
	file_descriptor fd;
};

struct standard_handle_input: public tinfra::input_stream {
    standard_handle_input();
    ~standard_handle_input();

    void close();
    int read(char* dest, int size);
};

struct standard_handle_output: public tinfra::output_stream {
    standard_handle_output(int where);
    ~standard_handle_output();

    using tinfra::output_stream::write;
    
    void close();
    int write(const char* data, int size);
    void sync();
private:
    bool fd_;
};

} // end namespace tinfra::posix

extern posix::standard_handle_input  in;
extern posix::standard_handle_output out;
extern posix::standard_handle_output err;

} // end namespace tinfra

#endif // tinfra_posix_stream_h_included

