//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include <stdexcept>
#include <string>

//#include <iostream>
//#include <sstream>

//#include "tinfra/runtime.h"

//#include "tinfra/aio.h"
//#include "tinfra/aio_net.h"
//#include "tinfra/connection_handler.h"
#include "tinfra/trace.h"

#include "protocol_aio_adapter.h"

namespace tinfra {
namespace aio {

//
// protocol_aio_adapter::buffer_impl
//

class protocol_aio_adapter::buffer_impl {
	std::string contents_;
public:
	void put(tstring const& b)
	{
		contents_.append(b.data(), b.size());
	}

	tstring get_contents() const {
		return tstring(contents_);
	}
	
	
	bool empty() const { 
		return contents_.size() == 0; 
	}
	
	void consume(size_t size)
	{
		contents_.erase(0, size);
	}
};

//
// protocol_aio_adapter::reader_impl
//

class protocol_aio_adapter::reader_impl {
	protocol_aio_adapter::buffer_impl buffer_;
	protocol_aio_adapter& parent_;
	
	bool eof_signaled_;
	bool eof_readed_;
public:
	reader_impl(protocol_aio_adapter& parent): 
		parent_(parent),		
		eof_signaled_(false),
		eof_readed_(false)
	{}
	
	void handle_reading(stream* c)
	{
		// TODO: recheck this condition! ?
		while( true ) {
			if( !buffer_.empty() )
				consume_buffer();
			if( read_next_chunk(c) <= 0 )
				break;
		}
	}
	bool is_waiting() const
	{
		return true;
	}
        
        void signal_eof()
        {            
            get_protocol().eof(buffer_.get_contents(), get_feedback_channel());
            eof_signaled_ = true;
            buffer_.consume(buffer_.get_contents().size());
        }
        
private:
	protocol& get_protocol() { return parent_.protocol_; }
	stream*   get_feedback_channel() { return parent_.get_feedback_channel(); }
	
	void consume_buffer() {
		int accepted = 0;
		while( ! buffer_.empty() ) {
			 accepted = get_protocol().process_input(buffer_.get_contents(), get_feedback_channel());
			 if( accepted == 0 ) 				 
				 break; // not nough data, try later
			 buffer_.consume(accepted);
		}
		maybe_signal_eof(accepted);
	}
	
	void maybe_signal_eof(int last_accepted)
	{
		// there are two conditions for signalling EOF
		// 1. buffer is empty and is EOF signaled
		// 2. EOF is readed and last accepted bytes == 0, 
		//    (means protocol expects something but it will never arrive)
		const bool clean_eof = buffer_.empty() && eof_readed_;
		const bool premature_eof = eof_readed_ && last_accepted == 0;
		if( clean_eof || premature_eof ) {
			signal_eof();
		}
	}
        
        
	int read_next_chunk(stream* channel)
	{
		if( eof_readed_ )
			return 0;
		char tmp[1024];
		int readed;
		try {
			readed = channel->read(tmp, sizeof(tmp));
		} catch( tinfra::io::would_block&) {
			// TODO: again this exception is bad design
			return -1;
		}
                TINFRA_TRACE_VAR(readed);
		if( readed == 0 ) {
		    eof_readed_ = true;
		    return 1;
		}
		buffer_.put(tinfra::tstring(tmp, readed));
		return readed;
	}
};

//
// protocol_aio_adapter::writer_impl
//

class protocol_aio_adapter::writer_impl {
	protocol_aio_adapter::buffer_impl buffer_;
	protocol_aio_adapter& parent_;
	
	bool waiting_write_;
	stream* base_channel_;
public:
	writer_impl(protocol_aio_adapter& parent): 
		parent_(parent),
		waiting_write_(false),
		base_channel_(0),
		stream_for_writing(*this)
	{}
	
	void set_base_channel(stream* c)
	{
		base_channel_ = c;
	}
	void handle_writing(stream* c)
	{
		write_buffered_data(c);
	}
	
	bool is_waiting() const {
		return !buffer_.empty();
	}
	
	stream* get_stream() {
		assert(base_channel_ != 0);
		return &stream_for_writing;
	}
private:
	class output_stream_adaptor: public tinfra::io::stream {
		protocol_aio_adapter::writer_impl& parent_;
	public:
		
		output_stream_adaptor(protocol_aio_adapter::writer_impl& p): parent_(p) {}
		
		virtual int write(const char* data, int size)
		{
			parent_.write_with_buffering(tinfra::tstring(data,size), parent_.base_channel_);
			return size;
		}
		
		// dummies
		virtual intptr_t native() const { return -1; }		
		virtual void release() { }		
		virtual void close() {}		
		virtual int seek(int, seek_origin) { throw std::logic_error("protocol_aio_adapter::output_stream_adaptor: seek() not supported"); }		
		virtual int read(char*, int) {  throw std::logic_error("protocol_aio_adapter::output_stream_adaptor: read() not supported"); }
		virtual void sync() { }
	};
	
	output_stream_adaptor stream_for_writing;
	
	size_t write_buffered_data(stream* channel)
	{
		while( !buffer_.empty() ) {
			int r = write_no_buffer(buffer_.get_contents(), channel);
			if( r == 0 ) {
				return 0;
			}
			buffer_.consume(r);
		}
		return 1;
	}
	
	void write_with_buffering(tstring const& bytes, stream* channel)
	{
		if( !buffer_.empty() ) {
			// buffer already full, so append bytes to buffer;
			buffer_.put(bytes);
			return;
		}
		const size_t readed = write_no_buffer(bytes, channel);
		if( readed == bytes.size() ) {
			return;
		}
		const size_t remaining_size = bytes.size() - readed;
		
		// TODO: check how this behaves in allocation failure case
		buffer_.put( tinfra::tstring(bytes.data() + readed, remaining_size));
	}
	
	size_t write_no_buffer(tstring const& bytes, stream* channel)
	{
		try {
			int r = channel->write(bytes.data(), bytes.size());
			return r;
		} catch( tinfra::io::would_block&) {
			return 0;
		}
	}
};

//
// protocol_aio_adapter
//
protocol_aio_adapter::protocol_aio_adapter(protocol& p) :
	reader_(new reader_impl(*this)),
	writer_(new writer_impl(*this)),
	protocol_(p)
{
}

protocol_aio_adapter::~protocol_aio_adapter()
{
}
void protocol_aio_adapter::event(dispatcher& d, stream* channel, int event)
{
	TINFRA_TRACE_MSG("protocol_aio_adapter: event");
	writer_->set_base_channel(channel);
	if( (event & dispatcher::READ) == dispatcher::READ) {		
		reader_->handle_reading(channel);
	}
	if( (event & dispatcher::WRITE) == dispatcher::WRITE) {
		writer_->handle_writing(channel);
	}
	
	d.wait(channel, dispatcher::READ,  reader_->is_waiting());
	d.wait(channel, dispatcher::WRITE, writer_->is_waiting());
}

void protocol_aio_adapter::failure(dispatcher& d, stream* c, int error)
{
    writer_->set_base_channel(c);
    reader_->signal_eof();
    d.remove(c);
    //TINFRA_TRACE_MSG("protocol_aio_adapter: failure NOT IMPLEMENTED");
}

stream* protocol_aio_adapter::get_feedback_channel()
{
	return writer_->get_stream();
}

} } // end namespace tinfra::aio



