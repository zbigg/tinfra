#ifndef tinfra_logger_h_included
#define tinfra_logger_h_included

#include "trace.h"   // for tinfra::trace::location
#include "tstring.h" // for tstring
#include "stream.h"  // for tinfra::output_stream
#include <time.h>    // for time_t

namespace tinfra {

/** Tinfra logging idea
 * there 3 general log entry types:
 *   - public
 *   - major - internal
 *   - internal trace
 *
 * Public, is a lifetime&use case level information that
 * lands in public log like syslog.
 * That covers: final failures, info
 *
 * Major internal, is always logged to application log.
 * That covers: errors, verbose, warning.
 *
 * Internal trace is conditionally traced into application debug log.
 * That covers all above and from enabled tracers.
 */
/** Log informational message.
 * 
 * Default logger is used.
 */
void log_info(tstring const& m);
void log_info(tstring const& m, tinfra::trace::location const& loc);

/** Log warning message
  *
  * Warning should be logged in case of an 
  * unsafe, suspicious, deprecated event in 
  * program.
  *
  * Default logger is used to report message.
  */
void log_warning(tstring const& m);
void log_warning(tstring const& m, tinfra::trace::location const& loc);

/** Log error message
  *
  * System internal errors should be logged.
  *
  * For what you should use error:
  *  - system call, API returned error
  *  - expected file is not in place or is not
  *    readable
  *
  * I.e when problem is detected 
  *
  * For what you shouldn't use it:
  *  - User errors should be handled by UI)
  *  - Invalid XML file, incorrectly encoded message 
  *    is not an error, its a warning or failure
  *
  * (use tinfra::fail)
  *
  * [ Jeez, what i meant by this ? why logger shouldn't have a
  *   fail interface ? 
  *
  * Default logger is used to report message.
  */
void log_error(tstring const& m);
void log_error(tstring const& m, tinfra::trace::location const& loc);

/** Log system visible failure.
  *
  */
void log_fail(tstring const& m);
void log_fail(tstring const& m, tinfra::trace::location const& loc);

//
// OO logger interface 
//

struct log_handler;

enum log_level {
    FATAL,
    FAIL,
    ERROR,
    NOTICE,
    WARNING,
    INFO,
    TRACE
};

class logger {
    log_handler& handler;
    tstring      component;
    
public:
    logger();
    logger(tstring const& component);
    
    logger(log_handler& output);
    logger(tstring const& component, log_handler& output);
    
    ~logger();
    
    void trace(tstring const& message);
    void trace(tstring const& m, tinfra::trace::location const& loc);
    
    void info(tstring const& message);
    void info(tstring const& m, tinfra::trace::location const& loc);
    
    void warning(tstring const& message);
    void warning(tstring const& m, tinfra::trace::location const& loc);
    
    void error(tstring const& message);
    void error(tstring const& m, tinfra::trace::location const& loc);
    
    void fail(tstring const& message);
    void fail(tstring const& m, tinfra::trace::location const& loc);

    void log(log_level, tstring const& m, tinfra::trace::location const& loc);
};

struct log_record {
    log_level level;
    tstring   component;
    tstring   message;
    time_t    timestamp;
    tinfra::trace::location location;
};

struct log_handler {
    // 
    virtual ~log_handler();
    
    // usage interface
    virtual void log(log_record const& record) = 0;
    
    // singleton interface
    static log_handler& get_default();
    
    // override default log handler
    // 0 - restore defaule
    static void         set_default(log_handler*);
};

class generic_log_handler: public tinfra::log_handler {
    tinfra::output_stream& out;
public:
    generic_log_handler(tinfra::output_stream&);
    ~generic_log_handler();
    
    // TBD settings:
    //  - elements
    //  - date format
    //  - use thread
private:
    virtual void log(log_record const& record);
};

} // end namespace tinfra

#endif // include guard


