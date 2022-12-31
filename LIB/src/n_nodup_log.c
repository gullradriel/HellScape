/**\file n_nodup_log.c
 * generic no duplicate logging system
 *\author Castagnier Mickael
 *\date 2015-09-21
 */

/*! Feature test macro */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#define _GNU_SOURCE_WAS_NOT_DEFINED
#endif
#include <stdio.h>
#include <stdarg.h>
#ifdef _GNU_SOURCE_WAS_NOT_DEFINED
#undef _GNU_SOURCE
#endif

#include "nilorea/n_common.h"
#include "nilorea/n_log.h"
#include "nilorea/n_str.h"
#include "nilorea/n_list.h"
#include "nilorea/n_hash.h"

#include "nilorea/n_nodup_log.h"

/*! internal: no dup hash_table log save */
static HASH_TABLE *_n_nodup_table = NULL ;

/*!\fn init_nodup_log( int max )
 *\brief initialize the no duplicate logging system
 *\param max the max size of the internal hash table. Leave it to zero or
 negative to keep the internal value in use ( 1024 )
 *\return TRUE or FALSE */
int init_nodup_log( int max )
{
    if( _n_nodup_table != NULL )
    {
        n_log( LOG_ERR, "Could not allocate the internal hash table because it's already done" );
        return FALSE ;
    }
    if( max <= 0 )
        max = 1024 ;

    _n_nodup_table = new_ht( max );

    if( _n_nodup_table == NULL )
    {
        n_log( LOG_ERR, "Could not allocate the internal hash with size %d", max );
        return FALSE ;
    }
    else
    {
        n_log( LOG_DEBUG, "LOGGING: nodup system activated with a hash table of %d cells", max );
    }

    return TRUE ;
} /* init_nodup_log() */



/*!\fn int empty_nodup_table()
 * \brief Empty the nodup internal table
 * \return TRUE or FALSE
 */
int empty_nodup_table()
{
    __n_assert( _n_nodup_table, return FALSE );
    return empty_ht( _n_nodup_table );
} /* empty_nodup_table() */



/*!\fn int close_nodup_log()
 * \brief Empty nodup logtable and close the no duplicate logging session
 * \return TRUE or FALSE
 */
int close_nodup_log()
{
    __n_assert( _n_nodup_table, return FALSE );
    return destroy_ht( &_n_nodup_table );
} /* close_nodup_log() */


/*!\fn char *get_nodup_key( const char *file , const char *func , int line )
 *\brief internal, get a key for a log entry
 *\param file log source file
 *\param func log source
 *\param line log source
 *\return The key for the the log entry
 */
static char *get_nodup_key( const char *file, const char *func, int line )
{
    N_STR *nstr = NULL ;
    char *ptr = NULL ;
    nstrprintf( nstr, "%s%s%d", file, func, line );
    __n_assert( nstr, return NULL );
    ptr = nstr -> data ;
    Free( nstr );
    return ptr ;
} /* get_nodup_key */



/*!\fn char *get_nodup_indexed_key( const char *file , const char *func , const char *prefix , int line )
 *\brief internal, get a key for an indexed log entry
 *\param file log source file
 *\param func log source
 *\param line log source
 *\param prefix prefix for log entry
 *\return The key for the the log entry
 */
static char *get_nodup_indexed_key( const char *file, const char *func, const char *prefix, int line )
{
    N_STR *nstr = NULL ;
    char *ptr = NULL ;
    nstrprintf( nstr, "%s%s%s%d", file, func, prefix, line );
    __n_assert( nstr, return NULL );
    ptr = nstr -> data ;
    Free( nstr );
    return ptr ;
} /* get_nodup_key */




/*!\fn check_n_log_dup( const char *log , const char *file , const char *func , int line )
 * \brief check if a log was already done or not at the given line, func, file
 * \param log the line of log
 * \param file the file containing the log
 * \param func the name of calling func
 * \param line the line of the log
 * \return 0 if not existing 1 if existing with the same value , 2 if key exist
 and value is different
 */
int check_n_log_dup( const char *log, const char *file, const char *func, int line )
{
    HASH_NODE *node = NULL ;
    char *key = NULL ;

    /* check if the nopdup session is on, else do a normal n_log */
    if( !_n_nodup_table )
    {
        return 3 ;
    }


    key = get_nodup_key( file, func, line );

    __n_assert( key, return FALSE );

    node = ht_get_node( _n_nodup_table, key );

    Free( key );

    if( node )
    {
        if( strcmp( log, node -> data . string ) == 0 )
        {
            return 1 ;
        }
        return 2 ;
    }
    return 0 ;

} /* check_n_log_dup(...) */


/*!\fn check_n_log_dup_indexed( const char *log , const char *file , const char *func , int line , const char *prefix )
 * \brief check if a log was already done or not at the given line, func, file
 * \param log the line of log
 * \param file the file containing the log
 * \param func the name of calling func
 * \param line the line of the log
 * \param prefix prefix to subgroup logs
 * \return 0 if not existing 1 if existing with the same value , 2 if key exist
 and value is different
 */
int check_n_log_dup_indexed( const char *log, const char *file, const char *func, int line, const char *prefix )
{
    HASH_NODE *node = NULL ;
    char *key = NULL ;

    /* check if the nopdup session is on, else do a normal n_log */
    if( !_n_nodup_table )
    {
        return 3 ;
    }

    key = get_nodup_indexed_key( file, func, prefix, line );

    __n_assert( key, return FALSE );

    node = ht_get_node( _n_nodup_table, key );

    Free( key )

    if( node )
    {
        if( strcmp( log, node -> data . string ) == 0 )
        {
            return 1 ;
        }
        return 2 ;
    }
    return 0 ;
} /* check_nolog_dup_indexed() */



/*!\fn void _n_nodup_log( int LEVEL , const char *file , const char *func , int line , const char *format , ... )
 *\brief Logging function. log( level , const char *format , ... ) is a macro around _log
 *\param LEVEL Logging level
 *\param file File containing the emmited log
 *\param func Function emmiting the log
 *\param line Line of the log
 *\param format Format and string of the log, printf style
 */
void _n_nodup_log( int LEVEL, const char *file, const char *func, int line, const char *format, ... )
{
    HASH_NODE *node = NULL ;
    va_list args ;

    char *syslogbuffer = NULL ;

    va_start (args, format);
    vasprintf( &syslogbuffer, format, args );
    va_end( args );

    char *key = get_nodup_key( file, func, line );
    int is_dup = check_n_log_dup( syslogbuffer, file, func, line );

    switch( is_dup )
    {
    /* new log entry for file,func,line */
    case 0 :
        if( _n_nodup_table )
        {
            ht_put_string( _n_nodup_table, key, syslogbuffer );
        }
        _n_log( LEVEL, file, func, line, "%s", syslogbuffer );
        break ;

    /* exising and same log entry, do nothing (maybe latter we will add a timeout to repeat logging)  */
    case 1 :
        break ;
    /* existing but different entry. We have to refresh and log it one time*/
    case 2 :
        node = ht_get_node( _n_nodup_table, key );
        if( node && node -> data . string )
        {
            Free( node -> data . string );
            node -> data . string = syslogbuffer ;
        }
        _n_log( LEVEL, file, func, line, "%s", syslogbuffer );
        break ;
    /* no nodup session started, normal loggin */
    case 3 :
    default:
        _n_log( LEVEL, file, func, line, "%s", syslogbuffer );
        break ;
    }

    Free( syslogbuffer );
    Free( key );

} /* _n_nodup_log() */


/*!\fn void _n_nodup_log_indexed( int LEVEL , const char *prefix , const char *file , const char *func , int line , const char *format , ... )
 *\brief Logging function. log( level , const char *format , ... ) is a macro around _log
 *\param LEVEL Logging level
 *\param prefix prefix to subgroup logs
 *\param file File containing the emmited log
 *\param func Function emmiting the log
 *\param line Line of the log
 *\param format Format and string of the log, printf style
 */
void _n_nodup_log_indexed( int LEVEL, const char *prefix, const char *file, const char *func, int line, const char *format, ... )
{
    HASH_NODE *node = NULL ;
    va_list args ;

    char *syslogbuffer = NULL ;

    va_start (args, format);
    vasprintf( &syslogbuffer, format, args );
    va_end( args );

    char *key = get_nodup_indexed_key( file, func, prefix, line );
    int is_dup = check_n_log_dup_indexed( syslogbuffer, file, func, line, prefix );

    switch( is_dup )
    {
    /* new log entry for file,func,line */
    case 0 :
        if( _n_nodup_table )
        {
            ht_put_string( _n_nodup_table, key, syslogbuffer );
        }
        _n_log( LEVEL, file, func, line, "%s", syslogbuffer );
        break ;

    /* exising and same log entry, do nothing (maybe latter we will add a timeout to repeat logging)  */
    case 1 :
        break ;
    /* existing but different entry. We have to refresh and log it one time*/
    case 2 :
        node = ht_get_node( _n_nodup_table, key );
        if( node && node -> data . string )
        {
            Free( node -> data . string );
            node -> data . string = syslogbuffer ;
        }
        _n_log( LEVEL, file, func, line, "%s", syslogbuffer );
        break ;
    /* no nodup session started, normal loggin */
    case 3 :
    default:
        _n_log( LEVEL, file, func, line, "%s", syslogbuffer );
        break ;

    }

    Free( syslogbuffer );
    Free( key );

} /* _n_nodup_log_indexed() */



/*!\fn void dump_nodup_log( char *file )
 *\brief Dump the duplicate error log hash table in a file
 *\param file The path and filename to the dump file
 *\return TRUE or FALSE
 */
int dump_nodup_log( char *file )
{
    FILE *out = NULL ;
    HASH_NODE *hash_node = NULL ;
    __n_assert( file, return FALSE );
    __n_assert( _n_nodup_table, return FALSE );

    char *tmpfile = NULL ;
    n_log( LOG_DEBUG, "outfile:%s", file );
    strprintf( tmpfile, "%s.tmp", file );

    out = fopen( tmpfile, "wb" );
    __n_assert( out, return FALSE );

    if( _n_nodup_table )
    {
        for( unsigned long int it = 0 ; it < _n_nodup_table -> size ; it ++ )
        {
            list_foreach( list_node, _n_nodup_table -> hash_table[ it ] )
            {
                hash_node = (HASH_NODE *)list_node -> ptr ;
                fprintf( out, "%s\n", hash_node -> data . string );
            }
        }
    }
    fclose( out );

    char *cmd = NULL ;
    strprintf( cmd, "mv -f %s %s ; chmod 644 %s", tmpfile, file, file );
    n_log( LOG_DEBUG, "%s", cmd );
    system( cmd );

    Free( cmd );
    Free( tmpfile );

    return TRUE ;

} /* dump_nodup_log() */
