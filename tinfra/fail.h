#ifndef tinfra_fail_h_included
#define tinfra_fail_h_included

#include "tstring.h"
#include "trace.h" // for tinfra::trace::location

#include <stdexcept>

namespace tinfra {

/** Fail subprogram with message.
  *
  * This aborts subprogram in clean way:
  * - reports an error through default logger (log_errror)
  * - cleans up subprogram state by unwind caued by throwing 
  *    tinfra::failure(message)
  *
  * It should be used for first line of error reporting:
  *  - invalid user input/arguments
  *  - API returned error
  *  - invalid format received
  *
  * Registered diagnostic handlers are executed before
  * throwing exception.
  *
  * Failure lifetime:
  *  - spot: fail: 
  *       in original place of failure, specific error 
  *       example: 
  *           fail('failed to open a file 'XXX' for writing', 'permission denied(EPERM)')
  *
  *  - incatch & generalize fail: 
  *       in all intermediate places
  *       example: save_document routing cught above error and "refail";
  *            catch(failire& f) { fail("failed to save document", f) }
  *  - final catch           
  *      - catch and exit(): 
  *           catch (failure& f) { log_fail("...", f); exit(1); } 
  *      - catched: in final place (server loop): 'httpd: HTTP 500, failed handle request XXX: ...)'
  *           catch (failure& f) { log_fail("...", f); continue; }
  * 
  * Doesn't return - always throw exception.
  */
void fail(tstring const& m, tstring const& cause);
void fail(tstring const& m, tstring const& cause, tinfra::trace::location const&);

/** Subprogram failure.
 * 
 * Thrown to report that some subprogram (or whole process)
 * failed and cannot continue (and shouldn't) with processing
 * 
 * As a result, program should exit or next job should be 
 * taken for processing.
 *
 * [Reception of this exception doesn't mean program stats
 * is corrupted.]
 *
 */
struct failure: public std::runtime_error {
    failure(tstring const& message);
    failure(tstring const& message, tstring const& cause);
};

} // end namespace tinfra

#endif // include guard
