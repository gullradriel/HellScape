/**\file n_list.c
 *  Pointer list functions definitions
 *\author Castagnier Mickael
 *\version 2.0
 *\date 24/04/13
 */

#include "nilorea/n_common.h"
#include <pthread.h>
#include "nilorea/n_log.h"
#include "nilorea/n_list.h"



/*!\fn LIST *new_generic_list( int max_items )
 *\brief Initialiaze a generic list container to max_items pointers.
 *\param max_items Specify a max size for the list container, 0 or negative for unlimited lists.
 *\return a new LIST or NULL
 */
LIST *new_generic_list( int max_items )
{
    LIST *list = NULL ;

    Malloc( list, LIST, 1 );
    __n_assert( list, return NULL );

    if( max_items <= 0 )
        list -> nb_max_items = 0 ;
    else
        list -> nb_max_items = max_items ;

    list -> nb_items = 0 ;
    list -> start = list -> end = NULL ;

    return list ;
} /* new_generic_list */



/*!\fn LIST_NODE *new_list_node( void *ptr , void (*destructor)( void *ptr ) )
 *\brief Allocate a new node to link in a list.
 *\param ptr The pointer you want to put in the node.
 *\param destructor Pointer to the ptr type destructor function. Leave to NULL if there isn't.
 *\return A new node or NULL on error. Check error messages in DEBUG mode.
 */
LIST_NODE *new_list_node( void *ptr, void (*destructor)( void *ptr ) )
{
    LIST_NODE *node = NULL ;

    Malloc( node, LIST_NODE, 1 );
    __n_assert( node,  n_log( LOG_ERR, "Error allocating node for ptr %p", ptr ); return NULL );

    node -> ptr = ptr ;
    node -> destroy_func = destructor ;
    node -> next = node -> prev = NULL ;

    return node ;
} /* new_list_node(...) */



/*!\fn void *remove_list_node_f( LIST *list , LIST_NODE *node )
 *\brief Internal function called each time we need to get a node out of a list
 *\param list The list to pick in
 *\param node The node to remove in the list
 *\return The node or NULL
 */
void *remove_list_node_f( LIST *list, LIST_NODE *node )
{
    void *ptr = NULL ;

    __n_assert( list, n_log( LOG_ERR, "can't remove from NULL list" );        return NULL );
    __n_assert( list->start, n_log( LOG_ERR, "can't remove from NULL list->start" ); return NULL );
    __n_assert( list->end, n_log( LOG_ERR, "can't remove from NULL list->end" );   return NULL );
    __n_assert( node, n_log( LOG_ERR, "can't remove from NULL node" );        return NULL );

    ptr = node -> ptr ;
    if( node -> prev && node -> next )
    {
        node -> prev -> next = node -> next ;
        node -> next -> prev = node -> prev ;
    }
    else
    {
        if( node -> prev == NULL && node -> next )
        {
            node -> next -> prev = NULL ;
            list -> start = node -> next ;
        }
        else
        {
            if( node -> prev && node -> next == NULL )
            {
                node -> prev -> next = NULL ;
                list -> end = node -> prev ;
            }
            else
            {
                if( node -> prev == NULL && node -> next == NULL )
                {
                    /* removing last item */
                    list -> start = list -> end = NULL ;
                }
            }
        }
    }
    Free( node );
    list -> nb_items -- ;
    return ptr ;
} /* remove_list_node_f(...) */



/*!\fn int list_node_push( LIST *list , LIST_NODE *node )
 *\brief Add a filled node to the end of the list.
 *\param list An initilized list container. A null value will cause an error and a _log message.
 *\param node The node pointer you want to put in the list. A null value will cause an error and a _log message.
 *\return TRUE or FALSE
 */
int list_node_push( LIST *list, LIST_NODE *node )
{
    __n_assert( list, n_log( LOG_ERR, "invalid list: NULL" ); return FALSE );

    if( list -> nb_max_items > 0 && ( list -> nb_items >= list -> nb_max_items ) )
    {
        n_log( LOG_ERR, "list is full" );
        return FALSE ;
    }
    if( list -> end )
    {
        list -> end -> next = node ;
        node -> prev = list -> end ;
        list -> end = node ;
    }
    else
    {
        list -> start = list -> end = node ;
    }
    list -> nb_items ++ ;
    return TRUE ;
} /* list_node_push() */



/*!\fn LIST_NODE *list_node_pop( LIST *list )
 *\brief Get a LIST_NODE pointer from the end of the list
 *\param list An initilized list container. A null value will cause an error and a _log message.
 *\return A LIST_NODE *node or NULL. Check error messages in DEBUG mode.
 */
LIST_NODE *list_node_pop( LIST *list )
{
    __n_assert( list, n_log( LOG_ERR, "invalid list: NULL" ); return NULL );

    if( list -> nb_items == 0 || list -> end == NULL )
        return NULL ;

    LIST_NODE *nodeptr = NULL ;

    nodeptr = list -> end ;
    if( list -> end -> prev )
    {
        list -> end = list -> end -> prev ;
        list -> end -> next = NULL ;
    }
    list -> nb_items -- ;
    if( list -> nb_items == 0 )
        list -> start = list -> end = NULL ;

    return nodeptr ;
} /* list_node_pop() */



/*!\fn LIST_NODE *list_node_shift( LIST *list )
 *\brief Get a LIST_NODE pointer from the start of the list.
 *\param list An initilized list container. A null value will cause an error and a _log message.
 *\return A LIST_NODE *node or NULL. Check error messages in DEBUG mode.
 */
LIST_NODE *list_node_shift( LIST *list )
{
    __n_assert( list, n_log( LOG_ERR, "invalid list: NULL" ); return NULL );

    if( list -> nb_items == 0 || list -> start == NULL )
        return NULL ;

    LIST_NODE *nodeptr = NULL ;
    nodeptr = list -> start ;

    if( list -> start -> next )
    {
        list -> start = list -> start -> next ;
        list -> start -> prev = NULL ;
    }

    list -> nb_items -- ;

    if( list -> nb_items == 0 )
        list -> start = list -> end = NULL ;


    return nodeptr ;
} /* list_node_shift() */



/*!\fn int list_node_unshift( LIST *list , LIST_NODE *node )
 *\brief Add a pointer at the start of the list.
 *\param list An initilized list container. A null value will cause an error and a _log message.
 *\param node The node pointer you wan to put in the list. A null value will cause an error and a _log message.
 *\return TRUE or FALSE
 */
int list_node_unshift( LIST *list, LIST_NODE *node )
{
    __n_assert( list, n_log( LOG_ERR, "invalid list: NULL" ); return FALSE );

    if( list -> nb_max_items > 0 && ( list -> nb_items >= list -> nb_max_items ) )
    {
        n_log( LOG_ERR, "list is full" );
        return FALSE ;
    }
    if( list -> start )
    {
        link_node( node, list -> start );
        list -> start = node ;
    }
    else
    {
        list -> start = list -> end = node ;
    }
    list -> nb_items ++ ;

    return TRUE ;
} /* list_node_unshift() */



/*!\fn int list_push( LIST *list , void *ptr , void (*destructor)( void *ptr ) )
 *\brief Add a pointer to the end of the list.
 *\param list An initilized list container. A null value will cause an error and a _log message.
 *\param ptr The pointer you wan to put in the list. A null value will cause an error and a _log message.
 *\param destructor Pointer to the ptr type destructor function. Leave to NULL if there isn't.
 *\return TRUE or FALSE. Check error messages in DEBUG mode.
 */
int list_push( LIST *list, void *ptr, void (*destructor)( void *ptr ) )
{
    LIST_NODE *node = NULL ;

    __n_assert( list, n_log( LOG_ERR, "invalid list: NULL" ); return FALSE );
    __n_assert( ptr, n_log( LOG_ERR, "invalid ptr: NULL" );  return FALSE );

    if( list -> nb_max_items > 0 && ( list -> nb_items >= list -> nb_max_items ) )
    {
        n_log( LOG_ERR, "list is full" );
        return FALSE ;
    }

    node = new_list_node( ptr, destructor );
    __n_assert( node, n_log( LOG_ERR, "Couldn't allocate new node" ); return FALSE );

    list -> nb_items ++ ;

    if( list -> end )
    {
        list -> end -> next = node ;
        node -> prev = list -> end ;
        list -> end = node ;
    }
    else
    {
        list -> start = list -> end = node ;
    }
    return TRUE ;
} /* list_push( ... ) */



/*!\fn int list_push_sorted( LIST *list , void *ptr , int (*comparator)( const void *a , const void *b ) , void (*destructor)( void *ptr ) )
 *\brief Add a pointer sorted in the list , starting by the end of the list.
 *\param list An initilized list container. A null value will cause an error and a _log message.
 *\param ptr The pointer you wan to put in the list. A null value will cause an error and a _log message.
 *\param comparator A pointer to a function which take two void * pointers and return an int.
 *\param destructor Pointer to the ptr type destructor function. Leave to NULL if there isn't.
 *\return TRUE or FALSE. Check error messages in DEBUG mode.
 */
int list_push_sorted( LIST *list, void *ptr, int (*comparator)( const void *a, const void *b ), void (*destructor)( void *ptr ) )
{
    LIST_NODE *nodeptr = NULL ;

    __n_assert( list, n_log( LOG_ERR, "invalid list: NULL" ); return FALSE );
    __n_assert( ptr, n_log( LOG_ERR, "invalid ptr: NULL" );  return FALSE );

    if( list -> nb_max_items > 0 && ( list -> nb_items >= list -> nb_max_items ) )
    {
        n_log( LOG_ERR, "list is full" );
        return FALSE ;
    }

    if( list -> end )
    {
        nodeptr = list -> end ;
        while( nodeptr && ( comparator( ptr, nodeptr -> ptr ) < 0 ) )
            nodeptr = nodeptr -> prev ;

        if( !nodeptr )
        {
            /* It's the lower ranked element in the sort */
            list_unshift( list, ptr, destructor );
        }
        else
        {
            /* we have a match inside the list. let's insert the datas */
            LIST_NODE *node_next = nodeptr -> next  ;
            LIST_NODE *newnode = new_list_node( ptr, destructor );
            __n_assert( newnode, n_log( LOG_ERR, "Couldn't allocate new node" ); return FALSE );

            if( node_next )
            {
                link_node( newnode, node_next );
            }
            else
                list -> end = newnode ;

            link_node( nodeptr, newnode );
            list -> nb_items ++ ;
        }
    }
    else
        list_push( list, ptr, destructor );

    return TRUE ;
} /* list_push_sorted( ... ) */



/*!\fn int list_unshift( LIST *list , void *ptr , void (*destructor)( void *ptr ) )
 *\brief Add a pointer at the start of the list.
 *\param list An initilized list container. A null value will cause an error and a _log message.
 *\param ptr The pointer you wan to put in the list. A null value will cause an error and a _log message.
 *\param destructor Pointer to the ptr type destructor function. Leave to NULL if there isn't.
 *\return A void *ptr or NULL. Check error messages in DEBUG mode.
 */
int list_unshift( LIST *list, void *ptr, void (*destructor)( void *ptr ) )
{
    LIST_NODE *node = NULL ;

    __n_assert( list, n_log( LOG_ERR, "invalid list: NULL" ); return FALSE );
    __n_assert( ptr, n_log( LOG_ERR, "invalid ptr: NULL" );  return FALSE );

    if( list -> nb_max_items > 0 && ( list -> nb_items >= list -> nb_max_items ) )
    {
        n_log( LOG_ERR, "list is full" );
        return FALSE ;
    }
    node = new_list_node( ptr, destructor );
    __n_assert( node, n_log( LOG_ERR, "Couldn't allocate new node" ); return FALSE );

    list -> nb_items ++ ;

    if( list -> start )
    {
        link_node( node, list -> start );
        list -> start = node ;
    }
    else
    {
        list -> start = list -> end = node ;
    }

    return TRUE ;
} /* list_unshift_f(...) */



/*!\fn int list_unshift_sorted( LIST *list , void *ptr , int (*comparator)( const void *a , const void *b ) , void (*destructor)( void *ptr )  )
 *\brief Add a pointer sorted in the list , starting by the start of the list.
 *\param list An initilized list container. A null value will cause an error and a _log message.
 *\param ptr The pointer you wan to put in the list. A null value will cause an error and a _log message.
 *\param comparator A pointer to a function which take two void * pointers and return an int.
 *\param destructor Pointer to the ptr type destructor function. Leave to NULL if there isn't.
 *\return TRUE or FALSE. Check error messages in DEBUG mode.
 */
int list_unshift_sorted( LIST *list, void *ptr, int (*comparator)( const void *a, const void *b ), void (*destructor)( void *ptr ) )
{
    LIST_NODE *nodeptr = NULL ;

    __n_assert( list, n_log( LOG_ERR, "invalid list: NULL" ); return FALSE );
    __n_assert( ptr, n_log( LOG_ERR, "invalid ptr: NULL" );  return FALSE );

    if( list -> nb_max_items > 0 && ( list -> nb_items >= list -> nb_max_items ) )
    {
        n_log( LOG_ERR, "list is full" );
        return FALSE ;
    }

    if( list -> start )
    {
        nodeptr = list -> start ;
        while( nodeptr && ( comparator( ptr, nodeptr -> ptr ) > 0 ) )
            nodeptr = nodeptr -> next ;

        if( !nodeptr )
        {
            /* It's the higher ranked element in the sort */
            list_push( list, ptr, destructor );
        }
        else
        {
            /* we have a match inside the list. let's insert the datas */
            LIST_NODE *node_prev = nodeptr -> prev  ;
            LIST_NODE *newnode = new_list_node( ptr, destructor );
            __n_assert( newnode, n_log( LOG_ERR, "Couldn't allocate new node" ); return FALSE );

            if( node_prev )
            {
                link_node( node_prev, newnode );
            }
            else
                list -> start = newnode ;

            link_node( newnode, nodeptr );
            list -> nb_items ++ ;
        }
    }
    else
        list_unshift( list, ptr, destructor );

    return TRUE ;
} /* list_unshift_sorted(...) */



/*!\fn void *list_pop_f( LIST *list )
 *\brief Get a pointer from the end of the list
 *\param list An initilized list container. A null value will cause an error and a _log message.
 *\return A void *ptr or NULL. Check error messages in DEBUG mode.
 */
void *list_pop_f( LIST *list )
{
    LIST_NODE *nodeptr = NULL ;
    void *ptr = NULL ;

    __n_assert( list, n_log( LOG_ERR, "invalid list: NULL" ); return NULL );

    if( list -> nb_items == 0 || list -> end == NULL )
        return NULL ;

    nodeptr = list -> end ;
    ptr = nodeptr -> ptr ;

    if( list -> end -> prev )
    {
        list -> end = list -> end -> prev ;
        list -> end -> next = NULL ;
    }

    list -> nb_items -- ;
    if( list -> nb_items == 0 )
        list -> start = list -> end = NULL ;

    Free( nodeptr );

    return ptr ;
} /* list_pop_f( ... ) */



/*!\fn void *list_shift_f( LIST *list )
 *\brief Get a pointer from the start of the list.
 *\param list An initilized list container. A null value will cause an error and a _log message.
 *\return A void *ptr or NULL. Check error messages in DEBUG mode.
 */
void *list_shift_f( LIST *list )
{
    LIST_NODE *nodeptr = NULL ;
    void *ptr = NULL ;

    __n_assert( list, n_log( LOG_ERR, "invalid list: NULL" ); return NULL );

    if( list -> nb_items == 0 || list -> start == NULL )
        return NULL ;

    nodeptr = list -> start ;
    ptr = nodeptr -> ptr ;

    if( list -> start -> next )
    {
        list -> start = list -> start -> next ;
        list -> start -> prev = NULL ;
    }

    list -> nb_items -- ;

    if( list -> nb_items == 0 )
        list -> start = list -> end = NULL ;

    Free( nodeptr );

    return ptr ;
} /* list_shift_f(...)*/


/*!\fn LIST_NODE *list_search( LIST *list , void *ptr )
 *\brief search ptr in list
 *\param list The target list
 *\param ptr Searched pointer
 *\return The LIST_NODE *node or NULL
 */
LIST_NODE *list_search( LIST *list, void *ptr )
{
    __n_assert( list, return NULL );

    list_foreach( node, list )
    {
        if( node -> ptr == ptr )
            return node ;
    }
    return NULL ;
} /* list_search */



/*!\fn LIST_NODE list_search_with_f( LIST *list , int (*checkfunk)( void *ptr ) );
 *\brief search ptr in list
 *\param list The target list
 *\param checkfunk Function pointer who should return 1 if it matches or 0 if it's not
 *\return The LIST_NODE *node or NULL
 */
LIST_NODE *list_search_with_f( LIST *list, int (*checkfunk)( void *ptr ) )
{
    __n_assert( list, return NULL );

    list_foreach( node, list )
    {
        if( checkfunk( node -> ptr ) )
            return node ;
    }
    return NULL ;
} /* list_search */



/*!\fn int list_empty( LIST *list )
 *\brief Empty a LIST list of pointers.
 *\param list The list to empty. An already empty or null list will cause an error and a _log message.
 *\return TRUE or FALSE. Check error messages in DEBUG mode.
 */
int list_empty( LIST *list )
{
    LIST_NODE *node = NULL, *node_ptr = NULL ;

    __n_assert( list, n_log( LOG_ERR, "list is NULL" ); return FALSE );

    node = list -> start ;
    while( node )
    {
        node_ptr = node ;
        node = node -> next ;
        if( node_ptr -> destroy_func != NULL )
        {
            node_ptr -> destroy_func( node_ptr -> ptr );
        }
        Free( node_ptr );
    }
    list -> start = list -> end = NULL ;
    list -> nb_items = 0 ;
    return TRUE ;
} /* list_empty( ... ) */



/*!\fn int list_empty_with_f( LIST *list , void (*free_fnct)( void *ptr ))
 *\brief Empty a LIST list of pointers.
 *\param list The list to empty. An already empty or null list will cause an error and a _log message.
 *\param free_fnct a function ptr to a void func( void *ptr )
 *\return TRUE or FALSE. Check error messages in DEBUG mode.
 */
int list_empty_with_f( LIST *list,  void (*free_fnct)( void *ptr )  )
{
    LIST_NODE *node = NULL, *node_ptr = NULL ;

    __n_assert( list, n_log( LOG_ERR, "list is NULL" ); return FALSE  );

    node = list -> start ;
    while( node )
    {
        node_ptr = node ;
        node = node -> next ;
        free_fnct( node_ptr -> ptr );
        Free( node_ptr );
    }
    list -> start = list -> end = NULL ;
    list -> nb_items = 0 ;
    return TRUE ;
} /* list_empty_with_f */



/*!\fn int list_destroy( LIST **list )
 *\brief Empty and Free a list container. Call the destructor each time.
 *\param list The list to destroy. An already empty or null list will cause an error and a _log message.
 *\return TRUE or FALSE. Check error messages in DEBUG mode.
 */
int list_destroy( LIST **list )
{
    __n_assert( list&&(*list), n_log( LOG_ERR, "list already destroyed" ); return FALSE );
    list_empty( (*list) );
    Free( (*list ) );
    return TRUE ;
} /* free_list( ... ) */
