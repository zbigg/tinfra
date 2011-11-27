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
    this->log(tinfra::TRACE, message, TINFRA_NULL_SOURCE_LOCATION);
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

void generic_log_handler::log(log_record const& record)
{
    using tinfra::path::basename;
    using tinfra::get_exepath;
    
    std::ostringstream formatter;
    
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
    formatter << ": " << record.message << "\n";
            
    const std::string str = formatter.str();
    tinfra::err.write(str.data(), str.size());
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

