/**\file n_hash.h
 * Hash functions and table
 *\author Castagnier Mickael
 *\version 2.0
 *\date 16/03/2015
 */

#ifndef NILOREA_HASH_HEADER_GUARD
#define NILOREA_HASH_HEADER_GUARD

#ifdef __cplusplus
extern "C" {
#endif

/**\defgroup HASH_TABLE HASH TABLES: classic or trie tree hash_tables
  \addtogroup HASH_TABLE
  @{
  */

#if defined( __linux__ ) || defined( _AIX ) || defined( __sun )
#include <arpa/inet.h>
#include <string.h>
#else
#include <string.h>
#endif

#include <stdint.h>

#include "n_common.h"
#include "n_list.h"

#if defined(_MSC_VER)
#include <stdlib.h>
/*! compatibility with existing rot func */
#define ROTL32(x,y)     _rotl(x,y)
/*! compatibility with existing rot func */
#define ROTL64(x,y)     _rotl64(x,y)
/*! compatibility with existing func */
#define BIG_CONSTANT(x) (x)
#else
/*! missing ROTL fix 32bit */
#define ROTL32(x,y)     rotl32(x,y)
/*! missing ROTL fix 64bit */
#define ROTL64(x,y)     rotl64(x,y)
/*! missing ROTL constant */
#define BIG_CONSTANT(x) (x##LLU)
#endif /* if defined MSVC ... */



/*! value of integral type inside the hash node */
#define HASH_INT        1
/*! value of double type inside the hash node */
#define HASH_DOUBLE     2
/*! value of char * type inside the hash node */
#define HASH_STRING     4
/*! value of pointer type inside the hash node */
#define HASH_PTR        8
/*! value of unknow type inside the hash node */
#define HASH_UNKNOWN    16
/*! Murmur hash using hash key string, hash key numeric value, index table with lists of elements */
#define HASH_CLASSIC    128
/*! TRIE tree using hash key string */
#define HASH_TRIE       256


/*! union of the value of a node */
union HASH_VALUE
{
    /*! integral type */
    int    ival ;
    /*! double type */
    double fval ;
    /*! pointer type */
    void   *ptr ;
    /*! char *type */
    char   *string ;
};


/*! structure of a hash table node */
typedef struct HASH_NODE
{
    /*! string key of the node if any, else NULL */
    char *key ;
    /*! key id of the node if any */
    char key_id ;
    /*! numeric key of the node if any, else < 0 */
    unsigned long int hash_value ;
    /*! type of the node */
    int type ;
    /*! data inside the node */
    union HASH_VALUE data ;
    /*! destroy_func */
    void (*destroy_func)( void *ptr );
    /*! HASH_TRIE mode: does it have a value */
    int is_leaf ;
    /*! flag to mark a node for rehash */
    int need_rehash ;
    /*! HASH_TRIE mode: pointers to children */
    struct HASH_NODE **children ;
    /*! HASH_TRIE mode: size of alphabet and so size of children allocated array */
    uint32_t alphabet_length ;
} HASH_NODE ;


/*! structure of a hash table */
typedef struct HASH_TABLE
{
    /*! size of the hash table */
    unsigned long int size,
             /*! total number of used keys in the table */
             nb_keys ;
    /*! table's seed */
    uint32_t seed ;
    /*! HASH_CLASSIC mode: preallocated hash table */
    LIST **hash_table;
    /*! HASH_TRIE mode: Start of tree */
    HASH_NODE *root ;
    /*! HASH_TRIE mode: size of the alphabet */
    unsigned int alphabet_length ;
    /*! HASH_TRIE mode: offset to deduce to individual key digits */
    unsigned int alphabet_offset ;
    /*! hashing mode, murmurhash and classic HASH_MURMUR, or HASH_TRIE */
    unsigned int mode ;
    /*! get HASH_NODE at 'key' from table */
    HASH_NODE *(*ht_get_node)( struct HASH_TABLE *table, const char *key );
    /*! put an integer */
    int (*ht_put_int)( struct HASH_TABLE *table, const char *key, int val );
    /*! put a double */
    int (*ht_put_double)( struct HASH_TABLE *table, const char *key, double val );
    /*! put a a pointer */
    int (*ht_put_ptr)( struct HASH_TABLE *table, const char *key, void  *ptr, void (*destructor)( void *ptr ) );
    /*! put an char *string */
    int (*ht_put_string)( struct HASH_TABLE *table, const char *key, char  *val );
    /*! put an char *string pointer */
    int (*ht_put_string_ptr)( struct HASH_TABLE *table, const char *key, char  *val );
    /*! get an int from a key's node */
    int (*ht_get_int)( struct HASH_TABLE *table, const char *key, int *val );
    /*! get a double from a key's node */
    int (*ht_get_double)( struct HASH_TABLE *table, const char *key, double *val );
    /*! get a pointer from a key's node */
    int (*ht_get_ptr)( struct HASH_TABLE *table, const char *key, void  **val );
    /*! get a char *string from a key's node */
    int (*ht_get_string)( struct HASH_TABLE *table, const char *key, char  **val );
    /*! remove given's key node from the table */
    int (*ht_remove)( struct HASH_TABLE *table, const char *key );
    /*! search elements given an expression */
    LIST *(*ht_search)( struct HASH_TABLE *table, int (*node_is_matching)( HASH_NODE *node ) );
    /*! empty a hash table. char *strings are also freed */
    int (*empty_ht)( struct HASH_TABLE *table );
    /*! destroy a hash table*/
    int (*destroy_ht)( struct HASH_TABLE **table );
    /*! print table */
    void (*ht_print)( struct HASH_TABLE *table );
} HASH_TABLE ;

/*! Cast a HASH_NODE element */
#define hash_val( node , type )\
	( (node && node -> ptr) ? ( (type *)( ( (HASH_NODE *)node -> ptr )-> data . ptr) ) : NULL )

/*! ForEach macro helper (classic / old) */
#define ht_foreach( __ITEM_ , __HASH_  ) \
	if( !__HASH_ ) \
	{ \
		n_log( LOG_ERR , "Error in ht_foreach, %s is NULL" , #__HASH_ ); \
	} \
	else if( __HASH_ -> mode != HASH_CLASSIC ) \
	{ \
		n_log( LOG_ERR , "Error in ht_foreach( %s , %s ) unsupportted mode %d" , #__ITEM_ , #__HASH_  , __HASH_ -> mode ); \
	} \
	else \
	for( unsigned long int __hash_it = 0 ; __hash_it < __HASH_ -> size ; __hash_it ++ ) \
	for( LIST_NODE *__ITEM_ = __HASH_ -> hash_table[ __hash_it ] -> start ; __ITEM_ != NULL; __ITEM_ = __ITEM_ -> next )

/*! ForEach macro helper, reentrant (classic / old)  */
#define ht_foreach_r( __ITEM_ , __HASH_ , __ITERATOR_ ) \
	if( !__HASH_ ) \
	{ \
		n_log( LOG_ERR , "Error in ht_foreach, %s is NULL" , #__HASH_ ); \
	} \
	else if( __HASH_ -> mode != HASH_CLASSIC ) \
	{ \
		n_log( LOG_ERR , "Error in ht_foreach, %d is an unsupported mode" , __HASH_ -> mode ); \
	} \
	else \
	for( unsigned long int __ITERATOR_ = 0 ; __ITERATOR_ < __HASH_ -> size ; __ITERATOR_ ++ ) \
	for( LIST_NODE *__ITEM_ = __HASH_ -> hash_table[ __ITERATOR_ ] -> start ; __ITEM_ != NULL; __ITEM_ = __ITEM_ -> next )

/*! Cast a HASH_NODE element */
#define HASH_VAL( node , type )\
	( (node && node -> data . ptr) ? ( (type *)node -> data . ptr ) : NULL )

/*!  ForEach macro helper */
#define HT_FOREACH( __ITEM_ , __HASH_ , ... ) \
	{ \
		do \
		{ \
			if( !__HASH_ ) \
			{ \
				n_log( LOG_ERR , "Error in ht_foreach, %s is NULL" , #__HASH_ ); \
			} \
			else \
			{ \
				if( __HASH_ -> mode == HASH_CLASSIC ) \
				{ \
					int CONCAT(__ht_node_trie_func_macro_break_flag_classic,__LINE__) = 0 ; \
					for( unsigned long int __hash_it = 0 ; __hash_it < __HASH_ -> size ; __hash_it ++ ) \
					{ \
						for( LIST_NODE *__ht_list_node = __HASH_ -> hash_table[ __hash_it ] -> start ; __ht_list_node != NULL; __ht_list_node = __ht_list_node -> next ) \
						{ \
							HASH_NODE *__ITEM_ = (HASH_NODE *)__ht_list_node -> ptr ; \
							CONCAT(__ht_node_trie_func_macro_break_flag_classic,__LINE__) = 1 ; \
							__VA_ARGS__ \
							CONCAT(__ht_node_trie_func_macro_break_flag_classic,__LINE__) = 0 ; \
						} \
						if( CONCAT(__ht_node_trie_func_macro_break_flag_classic,__LINE__) == 1 ) \
							break ; \
					} \
				} \
				else if( __HASH_ -> mode == HASH_TRIE ) \
				{ \
					int CONCAT(__ht_node_trie_func_macro,__LINE__)( HASH_NODE *__ITEM_ ) \
					{ \
						if( !__ITEM_ ) return TRUE ; \
						int CONCAT(__ht_node_trie_func_macro_break_flag,__LINE__) = 1 ; \
						if( __ITEM_ -> is_leaf ) \
						{ \
							do \
							{ \
								__VA_ARGS__  \
								CONCAT(__ht_node_trie_func_macro_break_flag,__LINE__) = 0 ; \
							}while ( 0 ); \
						} \
						if( CONCAT(__ht_node_trie_func_macro_break_flag,__LINE__) == 1 ) return FALSE ; \
						for( uint32_t it = 0 ; it < __ITEM_ -> alphabet_length ; it++ ) \
						{ \
							if( CONCAT(__ht_node_trie_func_macro,__LINE__)( __ITEM_ -> children[ it ] ) == FALSE )  \
								return FALSE ; \
						} \
						return TRUE ; \
					} \
					CONCAT(__ht_node_trie_func_macro,__LINE__)( __HASH_ -> root ); \
				} \
				else \
				{ \
					n_log( LOG_ERR , "Error in ht_foreach, %d is an unsupported mode" , __HASH_ -> mode ); \
					break ; \
				} \
			} \
		}while( 0 ); \
	}

/*!  ForEach macro helper */
#define HT_FOREACH_R( __ITEM_ , __HASH_ , __ITERATOR , ... ) \
	{ \
		do \
		{ \
			if( !__HASH_ ) \
			{ \
				n_log( LOG_ERR , "Error in ht_foreach, %s is NULL" , #__HASH_ ); \
			} \
			else \
			{ \
				if( __HASH_ -> mode == HASH_CLASSIC ) \
				{ \
					int CONCAT(__ht_node_trie_func_macro_break_flag_classic,__LINE__) = 0 ; \
					for( unsigned long int __ITERATOR = 0 ; __ITERATOR < __HASH_ -> size ; __ITERATOR ++ ) \
					{ \
						for( LIST_NODE *__ht_list_node = __HASH_ -> hash_table[ __ITERATOR ] -> start ; __ht_list_node != NULL; __ht_list_node = __ht_list_node -> next ) \
						{ \
							HASH_NODE *__ITEM_ = (HASH_NODE *)__ht_list_node -> ptr ; \
							CONCAT(__ht_node_trie_func_macro_break_flag_classic,__LINE__) = 1 ; \
							__VA_ARGS__ \
							CONCAT(__ht_node_trie_func_macro_break_flag_classic,__LINE__) = 0 ; \
						} \
						if( CONCAT(__ht_node_trie_func_macro_break_flag_classic,__LINE__) == 1 ) \
							break ; \
					} \
				} \
				else if( __HASH_ -> mode == HASH_TRIE ) \
				{ \
					int CONCAT(__ht_node_trie_func_macro,__LINE__)( HASH_NODE *__ITEM_ ) \
					{ \
						if( !__ITEM_ ) return TRUE ; \
						int CONCAT(__ht_node_trie_func_macro_break_flag,__LINE__) = 1 ; \
						if( __ITEM_ -> is_leaf ) \
						{ \
							do \
							{ \
								__VA_ARGS__  \
								CONCAT(__ht_node_trie_func_macro_break_flag,__LINE__) = 0 ; \
							}while ( 0 ); \
						} \
						if( CONCAT(__ht_node_trie_func_macro_break_flag,__LINE__) == 1 ) return FALSE ; \
						for( uint32_t it = 0 ; it < __ITEM_ -> alphabet_length ; it++ ) \
						{ \
							if( CONCAT(__ht_node_trie_func_macro,__LINE__)( __ITEM_ -> children[ it ] ) == FALSE )  \
								return FALSE ; \
						} \
						return TRUE ; \
					} \
					CONCAT(__ht_node_trie_func_macro,__LINE__)( __HASH_ -> root ); \
				} \
				else \
				{ \
					n_log( LOG_ERR , "Error in ht_foreach, %d is an unsupported mode" , __HASH_ -> mode ); \
					break ; \
				} \
			} \
		}while( 0 ); \
	}

void MurmurHash3_x86_32  ( const void * key, int len, uint32_t seed, void * out );
void MurmurHash3_x86_128 ( const void * key, int len, uint32_t seed, void * out );
void MurmurHash3_x64_128 ( const void * key, int len, uint32_t seed, void * out );

char *ht_node_type( HASH_NODE *node );
HASH_NODE *ht_get_node( HASH_TABLE *table, const char *key );

HASH_TABLE *new_ht( unsigned long int size );
HASH_TABLE *new_ht_trie( unsigned int alphabet_size, unsigned int alphabet_offset );

int ht_get_double( HASH_TABLE *table, const char *key, double *val );
int ht_get_int( HASH_TABLE *table, const char *key, int *val );
int ht_get_ptr( HASH_TABLE *table, const char *key, void  **val );
int ht_get_string( HASH_TABLE *table, const char *key, char  **val );
int ht_put_double( HASH_TABLE *table, const char *key, double value );
int ht_put_int( HASH_TABLE *table, const char *key, int value );
int ht_put_ptr( HASH_TABLE *table, const char *key, void  *ptr, void (*destructor)(void *ptr ) );
int ht_put_string( HASH_TABLE *table, const char *key, char *string );
int ht_put_string_ptr( HASH_TABLE *table, const char *key, char *string );
int ht_remove( HASH_TABLE *table, const char *key );
void ht_print( HASH_TABLE *table );
LIST *ht_search( HASH_TABLE *table, int (*node_is_matching)( HASH_NODE *node ) );
int empty_ht( HASH_TABLE *table );
int destroy_ht( HASH_TABLE **table );

HASH_NODE *ht_get_node_ex( HASH_TABLE *table, unsigned long int hash_value );
int ht_put_ptr_ex( HASH_TABLE *table, unsigned long int hash_value, void  *val, void (*destructor)( void *ptr ) );
int ht_get_ptr_ex( HASH_TABLE *table, unsigned long int hash_value, void  **val );
int ht_remove_ex( HASH_TABLE *table, unsigned long int hash_value );

LIST *ht_get_completion_list( HASH_TABLE *table, const char *keybud, uint32_t max_results );

int is_prime( int nb );
int next_prime( int nb );

int ht_get_table_collision_percentage( HASH_TABLE *table );
int ht_get_optimal_size( HASH_TABLE *table );
int ht_resize( HASH_TABLE **table, unsigned int size );
int ht_optimize( HASH_TABLE **table );

/**
  @}
  */

#ifdef __cplusplus
}
#endif

#endif // header guard

