/**\file n_signals.h
 *  Signals general handling with stack printing, from https://gist.github.com/jvranish/4441299
 \author Castagnier Mickael
 *\version 1.0
 *\date 08/11/2018
 */

#ifndef __N_SIGNALS__
#define __N_SIGNALS__

#ifdef __cplusplus
extern "C"
{
#endif


/**\defgroup SIGNALS SIGNALS: signals handling and stack printing
  \addtogroup SIGNALS
  @{
  */

/*! Size of the signal handler alternate stack */
#define SIGALTSTACK_SIZE 65536
/*! Number of backtrace log lines */
#define MAX_STACK_FRAMES 32

/* install the backtrace handler */
void set_signal_handler( const char *progname );

/**
@}
*/

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __N_SIGNALS__ */
