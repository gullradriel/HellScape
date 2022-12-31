/**\file n_exceptions.c
 * ExceptionContextList management for C
 *\author Castagnier Mickael
 *\version 1.0
 *\date 20/11/09
 */

#include <pthread.h>
#include "nilorea/n_common.h"
#include "nilorea/n_log.h"
#include "nilorea/n_exceptions.h"


ExceptionContextList* __Exceptions = NULL;



/*!\fn void push_exception()
 *\brief Function to push an exception in the list
 */
void push_exception(void)
{
    ExceptionContextList *_new = ( ExceptionContextList * )malloc( sizeof( ExceptionContextList ) );

    if( _new )
    {
        _new->head = __Exceptions;
        __Exceptions = _new;
    }
    else
    {
        n_log( LOG_ERR, "Cannot even add a new exception context !!" );
    }
} /* push_exception() */



/*!\fn void pop_exception( int ex )
 *\brief Function to pop an exception in the list
 *\param ex Type of exception to pop
 */
void pop_exception( int ex )
{
    ExceptionContextList *head = NULL ;
    head = __Exceptions -> head;
    Free( __Exceptions );
    __Exceptions = head;

    if( ex != NO_EXCEPTION )
    {
        if( __Exceptions )
        {
            longjmp( __Exceptions -> context, ex );
        }
    }
} /* pop_exception */

