#include "xml_parser.h" // we implement this

namespace tinfra {

xml_writer_options::xml_writer_options()
{
	human_readable = true;
	short_string_inline = true;
	indentation_size = 4;
	indentation_character = ' ';
}

class xml_streamer: public xml_output_stream {
	tinfra::output_stream* out_;
public:
	xml_streamer(tinfra::output_stream* out, xml_writer_options const& opts): 
		out_(out),
		tag_nest_(0),
		in_start_tag_(false),
		xml_document_started_(false),
		options(opts)
	{
	}
	
	~xml_streamer()
	{
	}
	
	// xml_output_stream interface
	void write(xml_event const& ev)
	{
		switch( ev.type ) {
		case xml_event::START_ELEMENT:
			start_tag(ev.content, ev.attributes);
			break;
		case xml_event::END_ELEMENT:
			end_tag(ev.content);
			break;
		case xml_event::CDATA:
			char_data(ev.content);
			break;
		default:
			assert(false);
		}
	}
	
	// implementation details
private:
	unsigned         tag_nest_;
	bool             in_start_tag_;
	bool             xml_document_started_;
	xml_writer_options options;
	
	
	void start_document()
	{
		write_bytes("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
	}
		
	void start_tag(tstring const& tag_name, xml_event_arg_list const& args)
	{
		ensure_tag_start_closed();
		maybe_indent();
		
		write_character('<');
		write_bytes(tag_name);
		this->in_start_tag_ = true;
		for( int i = 0; i < args.size(); ++i ) {
			arg(args[i].name, args[i].value);
		}
	}
	
	void arg(tstring const& name, tstring const& value)
	{
		write_character(' ');
		write_bytes(name);
		write_bytes("=\"");
		// TODO it should be escaped!
		write_bytes(value);
		write_character('"');
	}
	
	void char_data(tstring const& content)
	{
		ensure_tag_start_closed();
		if( ! options.human_readable ) {
			// TODO it should be escaped!	
			write_bytes(content);
		} else {
			tstring::size_type pos = 0;
			do {
				maybe_indent();
				tstring::size_type const next_lf = content.find_first_of('\n',pos);
				if( pos == tstring::npos ) {  
					write_bytes(content.substr(pos));
					break;
				} 
				
				// TODO it should be escaped!
				write_bytes(content.substr(pos, next_lf - pos + 1));
				pos = next_lf + 1;	
			} while( pos < content.size());
		}
	}
	
	void end_tag(tstring const& name)
	{
		if( in_start_tag_ ) {
			write_bytes("/>");
			in_start_tag_ = false;
		} else {
			--tag_nest_;
			maybe_indent();
			write_bytes("</");
			write_bytes(name);
			write_character('>');
		}
		maybe_newline();
	}

	void ensure_tag_start_closed()
	{
	    if( !this->xml_document_started_ ) {
		start_document();
	    }
	    if( this->in_start_tag_ ) {
		write_character('>');
		maybe_newline();        
		this->in_start_tag_ = false;
		++this->tag_nest_;
	    }
	    //I(in_start_tag_ == false);
	}
	
	void maybe_newline()
	{
	    if( options.human_readable ) 
	    	    write_character('\n');
	}
	
	void maybe_indent()
	{
	    if( options.human_readable ) 
	    	    make_indentation(this->tag_nest_, options.indentation_size);
	}
	
	void make_indentation(int level, int indent_size)
	{
	    for(int i = 0; i < indent_size*level; ++i )
	    	    write_character(options.indentation_character);
	}
	
	void write_bytes(tstring const& data)
	{
		out_->write(data.data(), data.size());
	}
	void write_character(char c)
	{
		out_->write(&c, 1);
	}

};

std::auto_ptr<xml_output_stream> xml_stream_writer(tinfra::output_stream* in, xml_writer_options const& options)
{
	return std::auto_ptr<xml_output_stream>(new xml_streamer(in, options));
}

} // end namespace tinfra

