#include "logger.h" // we implement this

#include "stream.h" // for tinfra::err
#include "trace.h"  // for TINFRA_NULL_SOURCE_LOCATION
#include "path.h"   // for basename
#include "exeinfo.h" // for get_exepath
#include "thread.h"

#include <time.h>
#include <sstream>  // for ostringstream
#include <unistd.h> //for getpid, win32 defaund yourself

namespace tinfra {

//
// log_xxx routines
//

void log_info(tstring const& m) { logger().info(m); }
void log_info(tstring const& m, tinfra::trace::location const& loc) { logger().info(m,loc); }

void log_warning(tstring const& m) { logger().warning(m); }
void log_warning(tstring const& m, tinfra::trace::location const& loc) { logger().warning(m,loc); }

void log_error(tstring const& m) { logger().error(m); }
void log_error(tstring const& m, tinfra::trace::location const& loc) { logger().error(m,loc); }

void log_fail(tstring const& m) { logger().fail(m); }
void log_fail(tstring const& m, tinfra::trace::location const& loc) { logger().fail(m,loc); }

//
// logger implementation
//

logger::logger():
    handler(log_handler::get_default()),
    component("")
{
}
logger::logger(tstring const& c):
    handler(log_handler::get_default()),
    component(c)
{
}

logger::logger(log_handler& h):
    handler(h),
    component("")
{
}

logger::logger(tstring const& c, log_handler& h):
    handler(h),
    component(c)
{
}

logger::~logger()
{
}

void logger::log(log_level level, tstring const& m, tinfra::trace::location const& loc)
{
    log_record record;
    record.level = level;
    record.component = this->component;
    record.message = m;
    ::time(&record.timestamp);
    record.location = loc;
    
    this->handler.log(record);
}

void logger::trace(tstring const& message)
{
    this->log(TRACE, message, TINFRA_NULL_SOURCE_LOCATION);
}

void logger::trace(tstring const& message, tinfra::trace::location const& loc)
{
    this->log(TRACE, message, loc);
}

void logger::info(tstring const& message)
{
    this->log(tinfra::INFO, message, TINFRA_NULL_SOURCE_LOCATION);
}
void logger::info(tstring const& message, tinfra::trace::location const& loc)
{
    this->log(tinfra::INFO, message, loc);
}

void logger::warning(tstring const& message)
{
    this->log(tinfra::WARNING, message, TINFRA_NULL_SOURCE_LOCATION);
}
void logger::warning(tstring const& message, tinfra::trace::location const& loc)
{
    this->log(tinfra::WARNING, message, loc);
}

void logger::error(tstring const& message)
{
    this->log(tinfra::ERROR, message, TINFRA_NULL_SOURCE_LOCATION);
}
void logger::error(tstring const& message, tinfra::trace::location const& loc)
{
    this->log(tinfra::ERROR, message, loc);
}

void logger::fail(tstring const& message)
{
    this->log(tinfra::FAIL, message, TINFRA_NULL_SOURCE_LOCATION);
}
void logger::fail(tstring const& message, tinfra::trace::location const& loc)
{
    this->log(tinfra::FAIL, message, loc);
}

static const char* log_level_to_string(log_level level)
{
    switch( level ) {
    case FATAL:   return "FATAL";
    case FAIL:    return "FAIL";
    case ERROR:   return "ERROR";
    case NOTICE:  return "NOTICE";
    case WARNING: return "WARNING";
    case INFO:    return "INFO";
    case TRACE:   return "TRACE";
    default:      return "-";    
    }
}

//
// generic_log_handler
//
generic_log_handler::generic_log_handler(tinfra::output_stream& o):
    out(o)
{
}
generic_log_handler::~generic_log_handler()
{
}

static void print_log_header(std::ostream& formatter, log_record const& record)
{
    using tinfra::path::basename;
    using tinfra::get_exepath;
        // well hardcoded, but why not
    // YYYY-MM-DD HH:MM:SS name[pid] level(component::func:source:line): message
    
    // date
    const char* LOG_TIME_FORMAT = "%Y-%m-%d %H:%M:%S";
    {
        
        struct tm exploded_time;
        localtime_r(&record.timestamp, &exploded_time);
        char strtime_buf[256];
        strftime(strtime_buf, sizeof(strtime_buf), LOG_TIME_FORMAT, &exploded_time);
        formatter << strtime_buf << ' '; 
    }
            
    // process name
    formatter << basename(get_exepath());
    
    // pid
    const bool show_pid = true;
    const bool show_tid = false;
    if( show_pid || show_tid ) {
        formatter << '[';
        if( show_pid ) 
            formatter << getpid();
        if( show_pid && show_tid ) 
            formatter << '/';
        if( show_tid )
            formatter << "tid=" << tinfra::thread::thread::current().to_number();
        formatter << "] ";
    }
    
    // level
    formatter << log_level_to_string(record.level);
    
    if( !record.component.empty() || record.location.name != 0 || record.location.filename != 0 ) {
        formatter << "(";
    
        // component/func/source location or -
        if( !record.component.empty() ) {
            formatter << record.component;
        }
        if( record.location.name != 0 ) {
            if( !record.component.empty() )
                formatter << "::";
            formatter << record.location.name;
        }
        if( record.location.filename != 0 ) {
            formatter << ':' << record.location.filename << ':' << record.location.line; 
        }
        formatter << ')';
    }
    
    
    // message
    formatter << ": ";
    
}

static void print_maybe_multiline(tstring const& PS1, tstring const& PS2, tstring const& message, tinfra::output_stream& out)
{
    // TODO: implement "multiline behaviour"
    size_t start = 0;
    bool finished = false;
    do {
        if( start == message.size() ) break;
            
        size_t eol = message.find_first_of('\n', start);
        size_t len;
        if( eol == std::string::npos ) {
            if( start != message.size()-1 ) {
                len = std::string::npos;
                finished = true;
            } else {
                return;
            }
        } else {    
            size_t pi = eol;
            if( pi > start+1 && message[eol-1] == '\r' ) --pi;
            len = pi-start;
        }
        if( len != 0 ) {
            // TODO: create tinfra string builders
            std::ostringstream tmp;
            tmp << (start == 0 ? PS1: PS2) << message.substr(start, len) << std::endl;
            std::string const stmp = tmp.str();
            out.write(stmp.data(), stmp.size());
        }
        start = eol+1;
    } while( !finished );
}

static void print_maybe_multiline(tstring const& prefix, tstring const& message, tinfra::output_stream& out)
{
    print_maybe_multiline(prefix,prefix,message,out);
}

void generic_log_handler::log(log_record const& record)
{
    std::ostringstream formatter;
    print_log_header(formatter, record);           
    const std::string header = formatter.str();
    
    print_maybe_multiline( header, header, record.message, this->out );
}

//
// log_handler
//

static generic_log_handler cerr_log_handler(tinfra::err);

static log_handler* custom_log_handler = 0;

log_handler::~log_handler()
{
}

// singleton interface
log_handler& log_handler::get_default()
{
    if( custom_log_handler != 0 ){
        return *custom_log_handler;
        
    } else {
        return cerr_log_handler;
    }
}

void         log_handler::set_default(log_handler* h)
{
    custom_log_handler = h;
}

} // end namespace tinfra

