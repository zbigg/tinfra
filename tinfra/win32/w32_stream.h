//
// Copyright (c) 2010-2011, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_w32_stream_h_included
#define tinfra_w32_stream_h_included

#ifndef tinfra_stream_h_included
#error "this file shall not be directly included"
#endif

namespace tinfra {
namespace win32 {

typedef intptr_t handle_type;

class file_handle {
public:
	typedef tinfra::win32::handle_type handle_type;
	
	explicit file_handle(handle_type existing, bool own = true);
	~file_handle();
	
	void reset(intptr_t fd, bool own);
	void close();
	
	handle_type handle() const { return this->handle2; }
private:
	// non-copyable
	file_handle(const file_handle&);
	file_handle& operator=(const file_handle&);
	
	handle_type handle2;
	bool own;
};

class native_input_stream: public tinfra::input_stream {
public:
	explicit native_input_stream(handle_type existing, bool own = true);
	~native_input_stream();
	
	// input_stream implementation
	void close();
	int read(char* dest, int size);
private:
	file_handle fd;
};

class native_output_stream: public tinfra::output_stream {
public:
	explicit native_output_stream(handle_type existing, bool own = true);
	~native_output_stream();
	
	// output_stream implementation
	using tinfra::output_stream::write;
	void close();
	int write(const char* data, int size);
    	void sync();
private:
	file_handle fd;
};

struct standard_handle_input: public tinfra::input_stream {
    standard_handle_input();
    ~standard_handle_input();

    void close();
    int read(char* dest, int size);
};

struct standard_handle_output: public tinfra::output_stream {
    standard_handle_output(bool is_err);
    ~standard_handle_output();

    using tinfra::output_stream::write;
    
    void close();
    int write(const char* data, int size);
    void sync();
private:
    bool is_err_;
};

} // end namespace tinfra::win32

extern win32::standard_handle_input  in;
extern win32::standard_handle_output out;
extern win32::standard_handle_output err;

} // end namespace tinfra

#endif // tinfra_w32_stream_h_included

