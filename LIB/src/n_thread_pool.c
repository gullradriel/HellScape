/**\file n_thread_pool.c
 *  Thread pool functions
 *\author Castagnier Mickael
 *\version 1.0
 *\date 30/04/2014
 */

#include "nilorea/n_common.h"
#include "nilorea/n_log.h"
#include "nilorea/n_thread_pool.h"
#include "nilorea/n_time.h"

#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>



/*!\fn void *thread_pool_processing_function( void *param )
 * \brief Internal thread pool processing function
 * \param param Teh eventual param of a process
 * \return NULL when exiting
 */
void *thread_pool_processing_function( void *param )
{
    THREAD_POOL_NODE *node = (THREAD_POOL_NODE *)param ;

    int thread_state = 0 ;

    do
    {
        //n_log( LOG_DEBUG, "Thread pool processing func waiting" );
        /* Direct procs automatically post th_start */
        sem_wait( &node -> th_start );

        pthread_mutex_lock( &node -> lock );
        thread_state = node -> thread_state ;
        pthread_mutex_unlock( &node -> lock );

        if( thread_state == RUNNING_THREAD )
        {
            //n_log( LOG_DEBUG, "Thread pool running proc %p", node -> func );

            pthread_mutex_lock( &node -> lock );
            node -> state = RUNNING_PROC ;
            pthread_mutex_unlock( &node -> lock );

            if( node && node -> func )
            {
                if( node -> param )
                    node -> func( node -> param ) ;
                else
                    node -> func( NULL );
            }
            //n_log( LOG_DEBUG, "Thread pool end proc %p", node -> func );

            pthread_mutex_lock( &node -> lock );
            node -> func = NULL ;
            node -> param = NULL ;
            node -> state = IDLE_PROC ;
            int type = node -> type ;
            node -> type = -1 ;
            pthread_mutex_unlock( &node -> lock );

            /* DIRECT_PROC do not need to post th_end */
            if( type&SYNCED_PROC )
                sem_post( &node -> th_end );
        }
    }
    while( thread_state != EXITING_THREAD );

    //n_log( LOG_DEBUG, "Thread processing func %ld exiting", node -> thr );

    pthread_mutex_lock( &node -> lock );
    node -> thread_state = EXITED_THREAD ;
    pthread_mutex_unlock( &node -> lock );

    //n_log( LOG_DEBUG, "Thread %ld exited", node -> thr );

    pthread_exit( NULL );

    return NULL ;
} /* thread_pool_processing_function */



/*!\fn THREAD_POOL *new_thread_pool( int nbmaxthr , int nb_max_waiting )
 * \brief Create a new pool of nbmaxthr threads
 * \param nbmaxthr number of active threads in the pool
 * \param nb_max_waiting max number of waiting procs in the pool. Negative or zero value for no limit
 * \return NULL or a new trhead pool object
 */
THREAD_POOL *new_thread_pool( int nbmaxthr, int nb_max_waiting )
{
    THREAD_POOL *thread_pool = NULL ;

    Malloc( thread_pool, THREAD_POOL, 1 );
    if( !thread_pool )
        return NULL ;

    thread_pool -> max_threads = nbmaxthr ;
    thread_pool -> nb_max_waiting = nb_max_waiting ;
    thread_pool -> nb_actives = 0 ;

    thread_pool -> thread_list =  (THREAD_POOL_NODE **)malloc( nbmaxthr * sizeof( THREAD_POOL_NODE * ) );
    if( !thread_pool -> thread_list )
    {
        Free( thread_pool );
        return NULL ;
    }

    thread_pool -> waiting_list = new_generic_list( 0 );
    if( !thread_pool -> waiting_list )
    {
        n_log( LOG_ERR, "Unable to initialize wait list" );
        return NULL ;
    }

    pthread_mutex_init( &thread_pool -> lock, NULL );

    for( int it = 0 ; it < nbmaxthr ; it ++ )
    {
        Malloc( thread_pool -> thread_list[ it ], THREAD_POOL_NODE, 1 );
        thread_pool -> thread_list[ it ] -> type = -1 ;
        thread_pool -> thread_list[ it ] -> state = IDLE_PROC ;
        thread_pool -> thread_list[ it ] -> thread_state = RUNNING_THREAD ;

        if( sem_init( &thread_pool -> thread_list[ it ] -> th_start, 0, 0 ) == -1 )
        {
            n_log( LOG_ERR, "sem_init failed : %s on &thread_pool -> thread_list[ %d ] -> th_start\n", strerror ( errno ), it );
            return NULL;
        }
        if( sem_init( &thread_pool -> thread_list[ it ] -> th_end, 0, 0 ) == -1 )
        {
            n_log( LOG_ERR, "sem_init failed : %s on &thread_pool -> thread_list[ %d] -> th_end\n", strerror ( errno ), it );
            return NULL;
        }

        thread_pool -> thread_list[ it ] -> func = NULL ;
        thread_pool -> thread_list[ it ] -> param = NULL ;

        pthread_mutex_init( &thread_pool -> thread_list[ it ] -> lock, NULL );

        if( pthread_create( &thread_pool -> thread_list[ it ] -> thr, NULL, thread_pool_processing_function, (void *)thread_pool -> thread_list[ it ] ) != 0 )
        {
            n_log( LOG_ERR, "pthread_create failed : %s for it %d\n",  strerror ( errno ), it );
            return NULL;
        }
    }
    return thread_pool ;
} /* new_thread_pool */



/*!\fn int add_threaded_process( THREAD_POOL *thread_pool , void *(*func_ptr)(void *param) , void *param , int mode )
 *\brief add a function and params to a thread pool
 *\param thread_pool The target thread pool
 *\param func_ptr The function pointer to launch
 *\param param Eventual parameter struct to pass to the function
 *\param mode SYNCED_PROC: added to pool, but wait for a start call with other waiting procs. DIRECT_PROC: the function actually starts as quick as possible
 *\return TRUE or FALSE
 */
int add_threaded_process( THREAD_POOL *thread_pool, void *(*func_ptr)(void *param), void *param,  int mode )
{

    if( !thread_pool )
        return FALSE ;

    if( !thread_pool -> thread_list )
        return FALSE ;

    int it = 0 ;
            
    if( !(mode&NO_LOCK) ) pthread_mutex_lock( &thread_pool -> lock );

    while( it < thread_pool -> max_threads )
    {
        pthread_mutex_lock( &thread_pool -> thread_list[ it ] -> lock );
        if( thread_pool -> thread_list[ it ] -> thread_state == RUNNING_THREAD && thread_pool -> thread_list[ it ] -> state == IDLE_PROC )
        {
            break ;
        }
        pthread_mutex_unlock( &thread_pool -> thread_list[ it ] -> lock );
        it ++ ;
    }
    /* all thread are occupied -> test waiting lists. not holding node lock  because it > max_threads !!! */
    if( it >= thread_pool -> max_threads )
    {
        /* if already coming from queue, or if it should be part of a synced start, do not re-add && return FALSE  */
        if( mode&NOQUEUE || mode&SYNCED_PROC )
        {
            if( !(mode&NO_LOCK) ) pthread_mutex_unlock( &thread_pool -> lock );
            return FALSE ;
        }
        if( thread_pool -> nb_max_waiting <= 0 || ( thread_pool -> waiting_list -> nb_items < thread_pool -> nb_max_waiting ) )
        {
            THREAD_WAITING_PROC *proc = NULL ;
            Malloc( proc, THREAD_WAITING_PROC, 1 );
            proc -> func = func_ptr ;
            proc -> param = param ;
            list_push( thread_pool -> waiting_list, proc, free );
            //n_log( LOG_DEBUG, "Adding %p %p to waitlist", proc -> func, proc -> param );
            if( !(mode&NO_LOCK) ) pthread_mutex_unlock( &thread_pool -> lock );
            return TRUE ;
        }
        else
        {
            n_log( LOG_ERR , "Proc %p %p was dropped from waitlist because waitlist is full", func_ptr, param );
            if( !(mode&NO_LOCK) ) pthread_mutex_unlock( &thread_pool -> lock );
            return FALSE ;
        }
    }
    if( mode&DIRECT_PROC )
    {
        thread_pool -> thread_list[ it ] -> func = func_ptr ;
        thread_pool -> thread_list[ it ] -> param = param ;
        thread_pool -> thread_list[ it ] -> state = WAITING_PROC ;
        thread_pool -> thread_list[ it ] -> type = mode ;
        sem_post( &thread_pool -> thread_list[ it ] -> th_start );
    }
    else if( mode&SYNCED_PROC )
    {
        thread_pool -> thread_list[ it ] -> func = func_ptr ;
        thread_pool -> thread_list[ it ] -> param = param ;
        thread_pool -> thread_list[ it ] -> state = WAITING_PROC ;
        thread_pool -> thread_list[ it ] -> type = mode ;
    }
    else
    {
        n_log( LOG_ERR, "Unknown mode %d for thread %d", mode, it );
        pthread_mutex_unlock( &thread_pool -> thread_list[ it ] -> lock );
        if( !(mode&NO_LOCK) ) pthread_mutex_unlock( &thread_pool -> lock );
        return FALSE ;
    }
    pthread_mutex_unlock( &thread_pool -> thread_list[ it ] -> lock );
    if( !(mode&NO_LOCK) ) pthread_mutex_unlock( &thread_pool -> lock );
    return TRUE ;
} /* add_threaded_process */



/*!\fn int start_threaded_pool( THREAD_POOL *thread_pool )
 * \brief Launch the process waiting for exectution in the thread pool
 * \param thread_pool The thread pool to launche
 * \return TRUE or FALSE
 */
int start_threaded_pool( THREAD_POOL *thread_pool )
{
    if( !thread_pool )
        return FALSE ;

    if( !thread_pool -> thread_list )
        return FALSE ;

    int retval = TRUE ;

    pthread_mutex_lock( &thread_pool -> lock );
    for( int it = 0 ; it < thread_pool -> max_threads ; it ++ )
    {
        int to_run = 0 ;
        pthread_mutex_lock( &thread_pool -> thread_list[ it ] -> lock );
        if( (thread_pool -> thread_list[ it ] -> type&SYNCED_PROC) && thread_pool -> thread_list[ it ] -> state == WAITING_PROC )
        {
            to_run = 1 ;
        }
        pthread_mutex_unlock( &thread_pool -> thread_list[ it ] -> lock );
        if( to_run == 1 )
        {
            if( sem_post( &thread_pool -> thread_list[ it ] -> th_start ) != 0 )
            {

            }   retval = FALSE ;
        }
    }
    pthread_mutex_unlock( &thread_pool -> lock );

    return retval ;
} /* start_threaded_pool */



/*!\fn int wait_for_threaded_pool( THREAD_POOL *thread_pool , int delay )
 * \brief Wait for all the launched process in the thread pool to terminate
 * \param thread_pool The thread pool to wait
 * \param delay time between each check
 * \return TRUE or FALSE
 */
int wait_for_threaded_pool(  THREAD_POOL *thread_pool, int delay )
{
    if( !thread_pool )
        return FALSE ;

    if( !thread_pool -> thread_list )
        return FALSE ;

    int DONE = 0 ;

    //n_log( LOG_DEBUG, "Waiting for the waitlist of %p to be consumed", thread_pool );

    /* waiting to consume all the waiting list */
    while( thread_pool -> waiting_list -> nb_items > 0 )
    {
        refresh_thread_pool( thread_pool );
        u_sleep( delay );
    }

    //n_log( LOG_DEBUG, "Waiting for active process of %p to be terminated", thread_pool );
    /* waiting for all active procs to have terminated */
    while( !DONE )
    {
        DONE = 1 ;
        for( int it = 0 ; it < thread_pool -> max_threads ; it ++ )
        {
            int state = 0 ;

            pthread_mutex_lock( &thread_pool -> thread_list[ it ] -> lock );
            state = thread_pool -> thread_list[ it ] -> state ;
            pthread_mutex_unlock( &thread_pool -> thread_list[ it ] -> lock );
            if( state != IDLE_PROC )
            {
                DONE = 0 ;
                //n_log( LOG_DEBUG, "Thread id %d status is not IDLE: %d", it, state );
            }
        }
        u_sleep( delay );
        refresh_thread_pool( thread_pool );
    }

    return TRUE ;
}


/*!\fn int destroy_threaded_pool( THREAD_POOL **pool  ,int delay )
 * \brief delete a thread_pool, exit the threads and free the structs
 * \param pool The THREAD_POOL *object to kill
 * \param delay The THREAD_POOL *object to kill
 * \return TRUE or FALSE
 */
int destroy_threaded_pool( THREAD_POOL **pool, int delay )
{
    __n_assert( pool&&(*pool), return FALSE );
    __n_assert( (*pool) -> thread_list, return FALSE );

    int state= 0, DONE = 0 ;

    while( !DONE )
    {
        DONE = 0 ;
        pthread_mutex_lock( &(*pool) -> lock );
        for( int it = 0 ; it < (*pool) -> max_threads ; it ++ )
        {
            pthread_mutex_lock( &(*pool) -> thread_list[ it ] -> lock );
            state = (*pool) -> thread_list[ it ] -> state ;
            pthread_mutex_unlock( &(*pool) -> thread_list[ it ] -> lock );

            if( state == IDLE_PROC )
            {
                //n_log( LOG_DEBUG, "Posting EXITING to thread %ld", (*pool) -> thread_list[ it ] -> thr  );
                pthread_mutex_lock( &(*pool) -> thread_list[ it ] -> lock );
                (*pool) -> thread_list[ it ] -> thread_state = EXITING_THREAD ;
                sem_post( &(*pool) -> thread_list[ it ] -> th_start );
                pthread_mutex_unlock( &(*pool) -> thread_list[ it ] -> lock );
                DONE = 1 ;
            }
            else
            {
                //n_log( LOG_DEBUG, "thr %ld proc state %d thr state %d", (*pool) -> thread_list[ it ] -> thr,  (*pool) -> thread_list[ it ] -> state,(*pool) -> thread_list[ it ] -> thread_state  );
            }
        }
        pthread_mutex_unlock( &(*pool) -> lock );

        u_sleep( delay );
    }

    pthread_mutex_lock( &(*pool) -> lock );
    for( int it = 0 ; it < (*pool) -> max_threads ; it ++ )
    {
        pthread_join( (*pool) -> thread_list[ it ] -> thr, NULL );
        pthread_mutex_destroy( &(*pool) -> thread_list[ it ] -> lock );
        sem_destroy(  &(*pool) -> thread_list[ it ] -> th_start );
        sem_destroy(  &(*pool) -> thread_list[ it ] -> th_end );
        Free( (*pool) -> thread_list[ it ] );
    }
    Free( (*pool) -> thread_list );
    list_destroy( &(*pool) -> waiting_list );

    pthread_mutex_unlock( &(*pool) -> lock );
    pthread_mutex_destroy( &(*pool) -> lock );

    Free( (*pool) );

    return TRUE ;
} /* destroy_threaded_pool */




/*!\fn  int refresh_thread_pool( THREAD_POOL *thread_pool )
 * \brief try to add some waiting DIRECT_PROCs on some free thread slots, else do nothing
 * \param thread_pool The thread pool to refresh
 * \return TRUE or FALSE
 */
int refresh_thread_pool( THREAD_POOL *thread_pool )
{
    /* Trying to empty the wait list */
    int push_status = 0 ;

    __n_assert( thread_pool, return FALSE );
    __n_assert( thread_pool -> waiting_list, return FALSE );

    pthread_mutex_lock( &thread_pool -> lock );

    if( thread_pool -> waiting_list && thread_pool -> waiting_list -> start )
        push_status = 1 ;

    while( push_status == 1 )
    {
        LIST_NODE *node = thread_pool -> waiting_list -> start ;
        LIST_NODE *next_node = NULL ;
        if( node && node -> ptr )
        {
            THREAD_WAITING_PROC *proc = (THREAD_WAITING_PROC *)node -> ptr ;
            if( proc )
            {
                if( add_threaded_process( thread_pool, proc -> func, proc -> param, DIRECT_PROC|NOQUEUE|NO_LOCK ) == TRUE )
                {
                    THREAD_WAITING_PROC *procptr = NULL ;
                    next_node = node -> next ;
                    procptr = remove_list_node( thread_pool -> waiting_list, node, THREAD_WAITING_PROC );
                    //n_log( LOG_DEBUG, "waitlist: adding %p,%p to %p", procptr -> func, procptr -> param, thread_pool );
                    Free( procptr );
                    node = next_node ;
                }
                else
                {
                    push_status = 0 ;
                }
            }
        }
        else
        {
            push_status = 0 ;
        }
    } /* while( push_status == 1 ) */

    /* update statictics */
    thread_pool -> nb_actives = 0 ;
    for( int it = 0 ; it < thread_pool -> max_threads ; it ++ )
    {
        pthread_mutex_lock( &thread_pool -> thread_list[ it ] -> lock );
        if( thread_pool -> thread_list[ it ] -> state == RUNNING_PROC )
            thread_pool -> nb_actives ++ ;
        pthread_mutex_unlock( &thread_pool -> thread_list[ it ] -> lock );
    }
    pthread_mutex_unlock( &thread_pool -> lock );



    return TRUE ;
} /* refresh_thread_pool() */
