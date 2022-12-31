/**\file n_thread_pool.h
 *  Thread pool declaration
 *\author Castagnier Mickael
 *\version 1.0
 *\date 30/04/2014
 */

#ifndef NILOREA_THREAD_POOL_LIBRARY
#define NILOREA_THREAD_POOL_LIBRARY

#ifdef __cplusplus
extern "C"
{
#endif

/**\defgroup THREADS THREADS: tools to create and manage a thread pool
   \addtogroup THREADS
  @{
*/

#include "n_list.h"
#include <pthread.h>
#include <semaphore.h>


/*! processing mode for added func, synced start */
#define SYNCED_PROC 1
/*! processing mode for added func, direct start */
#define DIRECT_PROC 2
/*! processing mode for waiting_list: do not readd the work in queue */
#define NOQUEUE 4
/*! status of a thread which is waiting for some proc */
#define IDLE_PROC 8
/*! status of a thread who have proc waiting to be processed  */
#define WAITING_PROC 16
/*! status of a thread which proc is currently running */
#define RUNNING_PROC 32
/*! indicate that the pool is running and ready to use */
#define RUNNING_THREAD 64
/*! indicate that the pool is exiting, unfinished jobs will finish and the pool will exit the threads and enter the EXITED state*/
#define EXITING_THREAD 128
/*! indicate that the pool is off, all jobs have been consumed */
#define EXITED_THREAD 256
/*! if passed to add_threaded_process, skip main table lock in case we are in a func which is already locking it */
#define NO_LOCK 512

/*! A thread pool node */
typedef struct THREAD_POOL_NODE
{
    /*! function to call in the thread */
    void *(*func)(void *param);

    /*! if not NULL , passed as argument */
    void *param ;

    /*! SYNCED or DIRECT process start */
    int type ;
    /*! state of the proc , RUNNING_PROC when it is busy processing func( param) , IDLE_PROC when it waits for some func and param to process , WAITING_PROC when it has things waiting to be processed */
    int state ;
    /*! state of the managing thread , RUNNING_THREAD, EXITING_THREAD, EXITED_THREAD */
    int thread_state ;
    /*! thread id */
    pthread_t thr ;

    /*! starting semaphore */
    sem_t th_start,
          /*! ending semaphore */
          th_end ;
    /*! mutex to prevent mutual access of node parameters */
    pthread_mutex_t lock ;

} THREAD_POOL_NODE ;


/*! Structure of a trhead pool */
typedef struct THREAD_POOL
{
    /*! Dynamically allocated but fixed size thread array */
    THREAD_POOL_NODE **thread_list ;

    /*! Maximum number of running threads in the list */
    int max_threads ;
    /*! Maximum number of waiting procedures int the list, 0 or -1 for unlimited */
    int nb_max_waiting ;
    /*! number of threads actually doing a proc */
    int nb_actives ;

    /*! mutex to prevent mutual access of waiting_list parameters */
    pthread_mutex_t lock ;
    /*! Waiting list handling */
    LIST *waiting_list ;

} THREAD_POOL ;



/*! Structure of a waiting process item */
typedef struct THREAD_WAITING_PROC
{
    /*! function to call in the thread */
    void *(*func)(void *param);
    /*! if not NULL , passed as argument */
    void *param ;

} THREAD_WAITING_PROC ;


/* allocate a new thread pool */
THREAD_POOL *new_thread_pool( int nbmaxthr, int nb_max_waiting );
/* add a function to run in an available thread inside a pool */
int add_threaded_process( THREAD_POOL *thread_pool, void *(*func_ptr)(void *param), void *param, int mode );
/* tell all the waiting threads to start their associated process */
int start_threaded_pool( THREAD_POOL *thread_pool );
/* wait for all running threads to finish */
int wait_for_threaded_pool( THREAD_POOL *thread_pool, int delay );
/* destroy all running threads */
int destroy_threaded_pool( THREAD_POOL **thread_pool, int delay );
/* try to add some waiting process on some free thread slots, else do nothing */
int refresh_thread_pool( THREAD_POOL *thread_pool );

/**
@}
*/

#ifdef __cplusplus
}
#endif

#endif

