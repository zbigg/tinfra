#include "xml_writer.h" // we implement this

#include <algorithm>
#include <limits>

namespace tinfra {

// TODO it should be moved to some other compilation unit
xml_event xml_event::make_copy(tinfra::string_pool& pool) const
{
	xml_event result;
	result.type = this->type;
	result.content = pool.alloc(this->content);
	result.attributes.reserve(this->attributes.size());
	for( unsigned i = 0; i < this->attributes.size(); ++i ) {
		xml_event_arg arg;
		arg.name = pool.alloc( this->attributes[i].name );
		arg.value = pool.alloc( this->attributes[i].value );
		result.attributes.push_back(arg);
	}
	return result;
}

xml_writer_options::xml_writer_options()
{
	start_document = true;
	human_readable = true;
	short_string_inline = true;
	indentation_size = 4;
	indentation_character = ' ';
}

class xml_content_encoder {
	tstring data;
	tstring pattern;
	const tstring* replacement;
	int position;
public:
	xml_content_encoder(tstring const& _data, tstring const& _pattern, const tstring* _replacement):
		data(_data),
		pattern(_pattern),
		replacement(_replacement),
		position(0)
	{
	    assert(_data.size() < size_t(std::numeric_limits<int>::max()));
	}
	
	bool has_next() const {
		return this->position < int(this->data.size());
	}
	
	tstring next() {
		tstring current = this->data.substr(this->position);
		typedef tstring::size_type size_type;
		const size_type first_replacement_pos = current.find_first_of(this->pattern);
		if( first_replacement_pos == tstring::npos ) {
			this->position = this->data.size();
			return current;
		} else if ( first_replacement_pos == 0 ) {
			// find replacement
			this->position++;
			const char* which = std::find(this->pattern.begin(), this->pattern.end(), current[0]);
			assert(which != this->pattern.end());
			int index = (which - this->pattern.data());
			assert(index < int(this->pattern.size()));
			const tstring found_replacement = this->replacement[index];
			return found_replacement;
		} else {
			tstring result = current.substr(0, first_replacement_pos);
			this->position += first_replacement_pos;
			return result;
		}
	}
};

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
		for( unsigned i = 0; i < args.size(); ++i ) {
			arg(args[i].name, args[i].value);
		}
	}
	
	void arg(tstring const& name, tstring const& value)
	{
		write_character(' ');
		write_bytes(name);
		write_bytes("=\"");
		// TODO it should be escaped!
		write_argument_value(value);
		write_character('"');
	}
	
	void char_data(tstring const& content)
	{
		ensure_tag_start_closed();
		if( true || ! options.human_readable ) {
			// TODO it should be escaped!
			write_content_bytes(content);
		} else {
			tstring::size_type pos = 0;
			do {
				maybe_indent();
				tstring::size_type const next_lf = content.find_first_of('\n',pos);
				if( pos == tstring::npos ) {  
					write_content_bytes(content.substr(pos));
					break;
				} 
				
				// TODO it should be escaped!
				write_content_bytes(content.substr(pos, next_lf - pos + 1));
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
	    if( !this->xml_document_started_ && options.start_document ) {
		start_document();
		this->xml_document_started_ = true;
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
	
	void write_argument_value(tstring const& data)
	{
		// http://www.w3.org/TR/REC-xml/#NT-AttValue
		// says that in att value, we escape " ' < and &
		static const tstring patterns = "\"'<&";
		static const tstring replacements[] = {
			"&quot;", "&apos;", "&lt;", "&amp;"
		};
		xml_content_encoder encoder(data, patterns, replacements);
		while( encoder.has_next() ) {
			tinfra::tstring chunk = encoder.next();
			out_->write(chunk.data(), chunk.size());
		}
	}
	
	void write_content_bytes(tstring const& data)
	{
		// http://www.w3.org/TR/REC-xml/#dt-chardata
		// says that only < and & are escaped 
		// in normal text
		static const tstring patterns = "<&";
		static const tstring replacements[] = {
			"&lt;", "&amp;"
		};
		xml_content_encoder encoder(data, patterns, replacements); 
		while( encoder.has_next() ) {
			tinfra::tstring chunk = encoder.next();
			out_->write(chunk.data(), chunk.size());
		}
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

