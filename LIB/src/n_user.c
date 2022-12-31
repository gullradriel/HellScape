/**\file n_user.c USERS fonctions
*\author Castagnier Mickael
*\version 1.0
*\date 20/02/2006
*/

#include "nilorea/n_user.h"



/*!\fn N_USERLIST *userlist_new( int max )
 *\brief create a new N_USERLIST user list with 'max' users
 *\param max the maximum number of users in the list
 *\return a new N_USERLIST *list or NULL
 */
N_USERLIST *userlist_new( int max )
{
    N_USERLIST *ulist = NULL ;
    Malloc( ulist, N_USERLIST, 1 );
    __n_assert( ulist, return NULL );
    __n_assert( max > 0, return NULL );

    Malloc( ulist -> list, N_USER, max );
    __n_assert( ulist -> list, Free( ulist ) ; return NULL );

    ulist -> max = max ;
    ulist -> highter = -1;
    for( int it = 0; it < max ; it ++ )
    {
        ulist -> list[ it ] . state = 0 ;
        ulist -> list[ it ] . nb_rec_pos = 10 ;
        ulist -> list[ it ] . only_last_pos = 1 ;
        ulist -> list[ it ] . id = -1 ;
        Malloc( ulist -> list[ it ] . last_positions, VECTOR3D, 10 );
        ulist -> list[ it ] . netw_waitlist = new_generic_list( 0 );
        memset( ulist -> list[ it ] . position, 0, 3 * sizeof( double ) );
        ulist -> list[ it ] . netw  = NULL ;
        memset( ulist -> list[ it ] . name, 0, 1024 );
        ulist -> list[ it ] . netw  = NULL ;
    }

    init_lock( ulist -> user_rwbolt );

    return ulist ;
} /* userlist_new() */



/*!\fn int userlist_set_position_behavior( N_USERLIST *ulist, int id, int nb_rec_pos, int only_last_pos )
 *\brief set the position parameters for trajectory processing for user 'id'
 *\param ulist targeted N_USERLIST *ulist
 *\param id id of the user
 *\param nb_rec_pos number of positions updates kept in list
 *\param only_last_pos flag if we're only keeping the last position ( if set nb_rec_pos is ignored )
 *\return TRUE or FALSE
 */
int userlist_set_position_behavior( N_USERLIST *ulist, int id, int nb_rec_pos, int only_last_pos )
{
    __n_assert( ulist, return FALSE );
    __n_assert( nb_rec_pos < 1, return FALSE );
    __n_assert( only_last_pos < 0 || only_last_pos > 1, return FALSE );

    read_lock( ulist -> user_rwbolt );
    __n_assert( ulist -> list[ id ] . state == 0, return FALSE );
    unlock( ulist -> user_rwbolt );

    write_lock( ulist -> user_rwbolt );
    ulist -> list[ id ] . nb_rec_pos = nb_rec_pos ;
    ulist -> list[ id ] . only_last_pos = only_last_pos ;
    if( only_last_pos )
    {
        Realloc(  ulist -> list[ id ] . last_positions, VECTOR3D, 1 );
        if( ! ulist -> list[ id ] . last_positions )
        {
            n_log( LOG_ERR, "could not resize to only_last_pos VECTOR3D" );
        }
    }
    else
    {
        Realloc(  ulist -> list[ id ] . last_positions, VECTOR3D, nb_rec_pos );
        if( ! ulist -> list[ id ] . last_positions )
        {
            n_log( LOG_ERR, "could not resize to %d VECTOR3D", nb_rec_pos );
        }
        ulist -> list[ id ] . nb_rec_pos = 1 ;
    }
    __n_assert( ulist -> list[ id ] . last_positions, unlock( ulist -> user_rwbolt ); return FALSE );
    unlock( ulist -> user_rwbolt );

    return TRUE ;
} /* userlist_set_position_behavior() */



/*!\fn int userlist_add_user( N_USERLIST *ulist, NETWORK *netw )
 *\brief add an user to the list
 *\param ulist targeted N_USERLIST *ulist
 *\param netw associated NETWORK *network associated to the new use to create
 *\return the new user id or -1 on error
 */
int userlist_add_user( N_USERLIST *ulist, NETWORK *netw )
{
    __n_assert( ulist, return -1 );
    __n_assert( netw, return -1 );

    int it = -1;
    write_lock( ulist -> user_rwbolt );
    do
    {
        it++;
    }
    while( it < ulist -> max && ulist -> list[ it ] . state != 0);
    if( it < ulist -> max )
    {
        ulist -> list[ it ] . state = 1;
        ulist -> list[ it ] . netw = netw ;
        if( it > ulist -> highter )
            ulist -> highter = it;
        unlock( ulist -> user_rwbolt );
        return it ;
    }
    unlock( ulist -> user_rwbolt );
    return -1 ;
} /* userlist_add_user() */



/*!\fn int userlist_del_user( N_USERLIST *ulist, int id )
 *\brief delete an user from the list
 *\param ulist targeted N_USERLIST *ulist
 *\param id id of the user to delete
 *\return TRUE or FALSE
 */
int userlist_del_user( N_USERLIST *ulist, int id )
{
    __n_assert( ulist, return FALSE );
    __n_assert( id >= 0, return FALSE );

    write_lock( ulist -> user_rwbolt );
    if( id >  ulist -> max )
    {
        unlock( ulist -> user_rwbolt );
        return FALSE ;
    }
    if( ulist -> list[ id ] . state == 0 )
    {
        unlock( ulist -> user_rwbolt );
        return FALSE;
    }
    ulist -> list[ id ] . state = 0 ;
    ulist -> list[ id ] . nb_rec_pos = 1 ;
    ulist -> list[ id ] . only_last_pos = 1 ;
    ulist -> list[ id ] . id = -1 ;
    Realloc(  ulist -> list[ id ] . last_positions, VECTOR3D, 1 );
    if( ! ulist -> list[ id ] . last_positions )
    {
        n_log( LOG_ERR, "couldn't resize to 1 element" );
    }
    list_empty( ulist -> list[ id ] . netw_waitlist );
    memset( ulist -> list[ id ] . position, 0, 3 * sizeof( double ) );
    ulist -> list[ id ] . netw  = NULL ;
    memset( ulist -> list[ id ] . name, 0, 1024 );

    if( id >=  ulist -> highter )
    {
        int it =  id ;
        while( it >= 0 )
        {
            if( ulist -> list[ it ] . state == 1 )
            {
                ulist -> highter  = it ;
                break ;
            }
            it -- ;
        }
    }
    unlock( ulist -> user_rwbolt );
    return TRUE;
} /* userlist_del_user() */



/*!\fn int userlist_add_msg_to_ex( N_USERLIST *ulist, N_STR *msg, int mode, int id )
 *\brief add a N_STR *message to user list
 *\param ulist targeted N_USERLIST *ulist
 *\param msg network message to add
 *\param mode one of USERLIST_ALL , USERLIST_ONE , USERLIST_ALL_EXCEPT
 *\param id targeted user id to use (USERLIST_ONE) or to ignore (USERLIST_ALL_EXCEPT)
 *\return TRUE or FALSE
 */
int userlist_add_msg_to_ex( N_USERLIST *ulist, N_STR *msg, int mode, int id )
{
    __n_assert( ulist, return FALSE );
    __n_assert( msg, return FALSE );

    switch( mode )
    {
    case( USERLIST_ALL ):
        read_lock( ulist -> user_rwbolt );
        for( int it = 0 ; it <= ulist -> highter ; it ++ )
        {
            if( ulist -> list[ it ] . state == 1 )
                netw_add_msg( ulist -> list[ it ] . netw, nstrdup( msg ) );
        }
        unlock( ulist -> user_rwbolt );
        break;
    case( USERLIST_ALL_EXCEPT ):
        __n_assert( id >= 0, return FALSE );
        read_lock( ulist -> user_rwbolt );
        for( int it = 0 ; it <= ulist -> highter ; it ++ )
        {
            if( it == id )
                continue ;
            if( ulist -> list[ it ] . state == 1 )
                netw_add_msg( ulist -> list[ it ] . netw, nstrdup( msg ) );
        }
        unlock( ulist -> user_rwbolt );
        break;
    default:
    case( USERLIST_ONE ):
        __n_assert( id >= 0, return FALSE );

        read_lock( ulist -> user_rwbolt );
        netw_add_msg( ulist -> list[ id ] . netw, msg );
        unlock( ulist -> user_rwbolt );
        break;
    }
    return TRUE ;
} /* userlist_add_msg_to_ex() */



/*!\fn int userlist_add_msg_to( N_USERLIST *ulist, N_STR *msg, int id )
 *\brief add a N_STR *message to user list (USERLIST_ONE)
 *\param ulist targeted N_USERLIST *ulist
 *\param msg network message to add
 *\param id targeted user id
 *\return TRUE or FALSE
 */
int userlist_add_msg_to( N_USERLIST *ulist, N_STR *msg, int id )
{
    return userlist_add_msg_to_ex( ulist, msg, USERLIST_ONE, id );
} /* userlist_add_msg_to() */


/*!\fn int userlist_add_msg_to_all( N_USERLIST *ulist, N_STR *msg )
 *\brief add a N_STR *message to user list (USERLIST_ALL)
 *\param ulist targeted N_USERLIST *ulist
 *\param msg network message to add
 *\return TRUE or FALSE
 */
int userlist_add_msg_to_all( N_USERLIST *ulist, N_STR *msg )
{
    return userlist_add_msg_to_ex( ulist, msg, USERLIST_ALL, -1 );
} /* userlist_add_msg_to_all() */



/*!\fn int userlist_add_msg_to_all_except( N_USERLIST *ulist, N_STR *msg, int id )
 *\brief add a N_STR *message to user list except user 'id' (USERLIST_ALL_EXCEPT)
 *\param ulist targeted N_USERLIST *ulist
 *\param id user id to ignore
 *\param msg network message to add
 *\return TRUE or FALSE
 */
int userlist_add_msg_to_all_except( N_USERLIST *ulist, N_STR *msg, int id )
{
    return userlist_add_msg_to_ex( ulist, msg, USERLIST_ALL_EXCEPT, id );
} /* userlist_add_msg_to_all_except() */



/*!\fn int userlist_destroy( N_USERLIST **ulist )
 *\brief destroy and free a N_USERLIST *userlist
 *\param ulist targeted N_USERLIST *list to destroy
 *\return TRUE or FALSE
 */
int userlist_destroy( N_USERLIST **ulist )
{
    __n_assert( ulist&&(*ulist), return FALSE );

    write_lock( (*ulist) -> user_rwbolt );
    for( int it = 0 ; it < (*ulist)  -> max ; it ++ )
    {
        Free( (*ulist) -> list[it ] . last_positions );
        list_destroy( &(*ulist) -> list[it ] . netw_waitlist );
    }
    Free( (*ulist) -> list );
    unlock( (*ulist) -> user_rwbolt );

    pthread_rwlock_destroy( &(*ulist) -> user_rwbolt );
    Free( (*ulist) );

    return FALSE ;
} /* userlist_destroy() */



/*!\fn int userlist_user_add_waiting_msg( N_USERLIST *ulist, int id, N_STR *netmsg )
 *\brief add a newtork message to specified user 'id'
 *\param ulist targeted N_USERLIST *list
 *\param id targeted user id
 *\param netmsg network message to add
 *\return TRUE or FALSE
 */
int userlist_user_add_waiting_msg( N_USERLIST *ulist, int id, N_STR *netmsg )
{
    __n_assert( ulist, return FALSE );
    __n_assert( netmsg, return FALSE );
    __n_assert( id >= 0, return FALSE );

    int ret = FALSE ;

    read_lock( ulist -> user_rwbolt );
    if( id <= ulist -> highter )
    {
        if( ulist -> list[ id ] . state == 1 )
        {
            ret = list_push( ulist -> list[ id ] . netw_waitlist, netmsg, NULL );
        }
    }
    unlock( ulist -> user_rwbolt );
    return ret ;
} /* userlist_user_add_waiting_msg() */



/*!\fn int userlist_user_send_waiting_msgs( N_USERLIST *ulist, int id )
 *\brief send all waiting messages in user 'id' waiting list
 *\param ulist targeted N_USERLIST *list
 *\param id targeted user id
 *\return TRUE or FALSE
 */
int userlist_user_send_waiting_msgs( N_USERLIST *ulist, int id )
{
    __n_assert( ulist, return FALSE );
    __n_assert( id >= 0, return FALSE );

    read_lock( ulist -> user_rwbolt );
    if( id <= ulist -> highter )
    {
        if( ulist -> list[ id ] . state == 1 )
        {
            list_foreach( node, ulist -> list[ id ] . netw_waitlist )
            {
                N_STR *netmsg = (N_STR *)node -> ptr ;
                netw_add_msg( ulist -> list[ id ] . netw, netmsg );
            }
            list_empty( ulist -> list[ id ] . netw_waitlist );
        }
    }
    unlock( ulist -> user_rwbolt );
    return TRUE ;
} /* userlist_user_send_waiting_msgs() */


/*!\fn int userlist_send_waiting_msgs( N_USERLIST *ulist )
 *\brief send all waiting messages ofr each user of the lsit
 *\param ulist targeted N_USERLIST *list
 *\return TRUE or FALSE
 */
int userlist_send_waiting_msgs( N_USERLIST *ulist )
{
    __n_assert( ulist, return FALSE );

    read_lock( ulist -> user_rwbolt );
    for( int id = 0 ; id <= ulist -> highter ; id ++ )
    {
        if( ulist -> list[ id ] . state == 1 )
        {
            list_foreach( node, ulist -> list[ id ] . netw_waitlist )
            {
                N_STR *netmsg = (N_STR *)node -> ptr ;
                netw_add_msg( ulist -> list[ id ] . netw, netmsg );
            }
            list_empty( ulist -> list[ id ] . netw_waitlist );
        }
    }
    unlock( ulist -> user_rwbolt );
    return TRUE ;
} /* userlist_send_waiting_msgs() */
