#ifndef tinfra_logger_h_included
#define tinfra_logger_h_included

namespace tinfra {

/** Log informational message.
 * 
 * Default logger is used.
 */
void inform(tstring const& m);

/** Log warning message
  *
  * Warning should be logged in case of an 
  * unsafe, suspicious, deprecated event in 
  * program.
  *
  * Default logger is used to report message.
  */
void warning(tstring const& m);

/** Log error message
  *
  * System internal errors should be logged.
  *
  * For what you should use error:
  *  - system call, API returned error
  *  - expected file is not in place or is not
  *    readable
  *
  * For what you shouldn't use it:
  *  - User errors should be handled by UI)
  *  - Invalid XML file, incorrectly encoded message 
  *    is not an error, its a warning or failure
  *
  * (use tinfra::fail)
  * 
  * Default logger is used to report message.
  */
void error(tstring const& m);

struct logger {
    void trace(tstring const& message) = 0;
    void inform(tstring const& message) = 0;
    void warning(tstring const& message) = 0;
    void error(tstring const& message) = 0; 
};

} // end namespace tinfra

#endif // include guard


