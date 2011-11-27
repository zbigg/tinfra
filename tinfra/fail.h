#ifndef tinfra_fail_h_included
#define tinfra_fail_h_included

#include <stdexcept>
#include "tstring.h"

namespace tinfra {

/** Fail program with message.
  *
  * This aborts subprogram in clean way:
  * - reports a failure through default logger
  * - cleans up subprogram state by unwind caued by throwing 
  *    tinfra::failure(message)
  *
  * It should be used for use-case level errors
  *  - invalid user input/arguments
  *  - invalid message received
  *
  * Registered diagnostic handlers are executed before
  * throwing exception.
  * 
  * Doesn't return - always throw exception.
  */
void fail(tstring const& m);

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
    failure(const char* message);
};

//
// implementation
//

inline failure::failure(const char* message): 
    std::runtime_error(message) 
{
}

} // end namespace tinfra

#endif // include guard
