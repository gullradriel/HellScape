/**\file n_time.c
 *  common time function
 *\author Castagnier Mickael
 *\version 1.0
 *\date 24/03/05
 */

#include "nilorea/n_time.h"


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#if defined(__windows__)
/*!\fn void u_sleep( __int64 usec)
 *\brief u_sleep for windows
 *\param used Number of usec to sleep
 */
void u_sleep( __int64 usec)
{
    HANDLE timer;
    LARGE_INTEGER ft;

    ft.QuadPart = -(10*usec); // Convert to 100 nanosecond interval, negative value indicates relative time

    timer = CreateWaitableTimer(NULL, TRUE, NULL);
    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);
}
#else
/*!\fn void u_sleep( unsigned int usec)
 *\brief wrapper around usleep for API consistency
 *\param usec Number of usec to sleep
 */
void u_sleep( unsigned int usec )
{
    usleep( usec );
}
#endif



/*!\fn void PAUSE()
 *\brief make a pause in a terminal
 */
void PAUSE( void )
{

    char k[ 2 ] = "" ;

    printf( "Press a enter to continue..." );

    fflush( stdin );
    scanf( "%c", k );

    printf( "\n" );
} /* PAUSE(...) */



/*!\fn int start_HiTimer( N_TIME *timer )
 *\brief Initialize or restart from zero any N_TIME HiTimer
 *\param timer Any N_TIMER *timer you wanna start or reset
 *\return TRUE or FALSE
 */
int start_HiTimer( N_TIME *timer )
{
    /* set delta to 0 */
    timer -> delta = 0;

#if defined( __windows__ )
    if( QueryPerformanceFrequency( ( LARGE_INTEGER * ) & timer -> freq ) == 0 )
        return FALSE ;
    if( QueryPerformanceCounter( &timer -> startTime ) == 0 )
        return FALSE ;
#else
    if( gettimeofday( &timer -> startTime, 0 ) != 0 )
        return FALSE ;
#endif

    return TRUE ;
} /* init_HiTimer(...) */



/*!\fn time_t get_usec( N_TIME  *timer )
 *\brief Poll any N_TIME HiTimer, returning usec, and moving currentTime to startTime
 *\param timer Any N_TIMER *timer you wanna poll
 *\return The elapsed number of usec for the given N_TIME *timer
 */
time_t get_usec( N_TIME *timer )
{
#ifdef __windows__
    QueryPerformanceCounter( ( LARGE_INTEGER * ) & timer -> currentTime );
    timer -> delta = 1000000 * ( timer -> currentTime . QuadPart - timer -> startTime . QuadPart ) / timer -> freq . QuadPart ;
    timer -> startTime = timer -> currentTime ;
#else
    gettimeofday( &timer -> currentTime, 0 );
    timer -> delta = ( timer -> currentTime . tv_sec   - timer -> startTime . tv_sec  ) * 1000000 + ( timer -> currentTime . tv_usec  - timer -> startTime . tv_usec );
    timer -> startTime . tv_sec  = timer -> currentTime . tv_sec ;
    timer -> startTime . tv_usec = timer -> currentTime . tv_usec ;
#endif

    return timer -> delta;
} /* get_usec( ... ) */



/*!\fn time_t get_msec( N_TIME  *timer )
 *\brief Poll any N_TIME HiTimer, returning msec, and moving currentTime to startTime
 *\param timer Any N_TIMER *timer you wanna poll
 *\return The elapsed number of msec for the given N_TIME *timer
 */
time_t get_msec( N_TIME *timer )
{
#ifdef __windows__
    QueryPerformanceCounter( ( LARGE_INTEGER * ) & timer -> currentTime );
    timer -> delta = 1000 * ( timer -> currentTime . QuadPart - timer -> startTime . QuadPart ) / timer -> freq . QuadPart ;
    timer -> startTime = timer -> currentTime ;
#else
    gettimeofday( &timer -> currentTime, 0 );
    timer -> delta = ( timer -> currentTime . tv_sec   - timer -> startTime . tv_sec  ) * 1000 + ( timer -> currentTime . tv_usec  - timer -> startTime . tv_usec ) / 1000 ;
    timer -> startTime . tv_sec  = timer -> currentTime . tv_sec ;
    timer -> startTime . tv_usec = timer -> currentTime . tv_usec ;
#endif

    return timer -> delta;
} /* get_msec(...) */



/*!\fn time_t get_sec( N_TIME  *timer )
 *\brief Poll any N_TIME HiTimer, returning sec, and moving currentTime to startTime
 *\param timer Any N_TIMER *timer you wanna poll
 *\return The elapsed number of sec for the given N_TIME *timer
 */
time_t get_sec( N_TIME *timer )
{
#ifdef __windows__
    QueryPerformanceCounter( ( LARGE_INTEGER * ) & timer -> currentTime );
    timer -> delta = ( timer -> currentTime . QuadPart - timer -> startTime . QuadPart ) / timer -> freq . QuadPart ;
    timer -> startTime = timer -> currentTime ;
#else
    gettimeofday( &timer -> currentTime, 0 );
    timer -> delta = ( timer -> currentTime . tv_sec   - timer -> startTime . tv_sec  )  + ( timer -> currentTime . tv_usec  - timer -> startTime . tv_usec ) / 1000000 ;
    timer -> startTime . tv_sec  = timer -> currentTime . tv_sec ;
    timer -> startTime . tv_usec = timer -> currentTime . tv_usec ;
#endif
    return timer -> delta;
}/* get_sec(...) */

