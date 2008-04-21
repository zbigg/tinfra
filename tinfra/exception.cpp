namespace tinfra {
    
void print_stacktrace(std::ostream& out, int ignore_frames)
{
    stacktrace_t stacktrace;
    get_stacktrace(stacktrace,ignore_frames);

    for( stacktrace_t::const_iterator i = stacktrace.begin(); i != stacktrace.end(); ++i ) {
		out << "0x" << std::setfill('0') << std::setw(sizeof(i->address)) << std::hex << i->address 
	          << "(" << i->symbol << ")" << std::endl;
    }
}
//
// generic_exception implementation
//

generic_exception::generic_exception(std::string const& message): _message(message) {
    populate_stacktrace(_stacktrace, 2);
    /*
    std::cerr << "exception: " << message << std::endl;
    for( stacktrace_t::const_iterator i = _stacktrace.begin(); i != _stacktrace.end(); ++i ) {
	std::cerr << "    at 0x" << std::setfill('0') << std::setw(8) << std::hex << (long)i->address 
	          << "(" << i->symbol << ")" << std::endl;
    }
    */
}

}