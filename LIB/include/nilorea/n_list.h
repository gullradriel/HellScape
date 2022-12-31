/**\file n_list.h
 *  List structures and definitions
 *\author Castagnier Mickael
 *\version 1.0
 *\date 24/04/13
 */

#ifndef N_GENERIC_LIST
#define N_GENERIC_LIST

#ifdef __cplusplus
extern "C"
{
#endif

/**\defgroup LIST LISTS: generic type list
   \addtogroup LIST
  @{
*/

#include "stdio.h"
#include "stdlib.h"


/*! Structure of a generic list node */
typedef struct LIST_NODE
{
    /*! void pointer to store */
    void *ptr ;

    /*! pointer to destructor function if any, else NULL */
    void (*destroy_func)(void *ptr );

    /*! pointer to the next node */
    struct LIST_NODE *next ;
    /*! pointer to the previous node */
    struct LIST_NODE *prev ;

} LIST_NODE ;



/*! Structure of a generic LIST container */
typedef struct LIST
{
    /*! number of item currently in the list */
    int nb_items ;
    /*! maximum number of item in the list. Unlimited 0 or negative */
    int nb_max_items ;

    /*! pointer to the start of the list */
    LIST_NODE *start ;
    /*! pointer to the end of the list */
    LIST_NODE *end ;

} LIST ;



/*! Macro helper for linking two nodes */
#define link_node( __NODE_1 , __NODE_2 ) \
   do \
   { \
      __NODE_2 -> prev = __NODE_1 ; \
      __NODE_1 -> next = __NODE_2 ; \
   } \
   while( 0 )

/*! ForEach macro helper */
#define list_foreach( __ITEM_ , __LIST_  ) \
   if( !__LIST_ ) \
   { \
      n_log( LOG_ERR , "Error in list_foreach, %s is NULL" , #__LIST_ ); \
   } \
   else \
   for( LIST_NODE *__ITEM_ = __LIST_ -> start ; __ITEM_ ; __ITEM_ = __ITEM_ -> next )

/*! Pop macro helper for void pointer casting */
#define list_pop( __LIST_ , __TYPE_ ) ( __TYPE_ *)list_pop_f( __LIST_ )
/*! Shift macro helper for void pointer casting */
#define list_shift( __LIST_ , __TYPE_ ) (__TYPE_ *)list_shift_f( __LIST_ )
/*! Remove macro helper for void pointer casting */
#define remove_list_node( __LIST_ ,__NODE_ , __TYPE_ ) (__TYPE_ *)remove_list_node_f( __LIST_ , __NODE_ )

/* initialize a list */
LIST *new_generic_list( int max_items );
/* create a new node */
LIST_NODE *new_list_node( void *ptr, void (*destructor)( void *ptr ) );
/* remove a node */
void *remove_list_node_f( LIST *list, LIST_NODE *node );
int list_node_push( LIST *list, LIST_NODE *node );
LIST_NODE *list_node_pop( LIST *list );
LIST_NODE *list_node_shift( LIST *list );
int list_node_unshift( LIST *list, LIST_NODE *node );

/* add a pointer at the end of the list */
int list_push( LIST *list, void *ptr, void (*destructor)( void *ptr ) ) ;
/* add a pointer sorted via comparator from the end of the list */
int list_push_sorted( LIST *list, void *ptr, int (*comparator)( const void *a, const void *b ), void (*destructor)( void *ptr ) );
/* put a pointer at the beginning of list */
int list_unshift( LIST *list, void *ptr, void (*destructor)( void *ptr ) );
/* put a point pointer sorted via comparator from the start to the end */
int list_unshift_sorted( LIST *list, void *ptr, int (*comparator)( const void *a, const void *b ), void (*destructor)( void *ptr ) );

/* get last ptr from list */
void *list_pop_f( LIST *list ) ;
/* get first ptr from list */
void *list_shift_f( LIST *list );

/* search ptr in list */
LIST_NODE *list_search( LIST *list, void *ptr );
/* search for data in list */
LIST_NODE *list_search_with_f( LIST *list, int (*checkfunk)( void *ptr ) );

/* empty the list */
int list_empty( LIST *list );
int list_empty_with_f( LIST *list, void (*free_fnct)( void *ptr ) );

/* free the list */
int list_destroy( LIST **list );

/**
@}
*/

#ifdef __cplusplus
}
#endif

#endif
