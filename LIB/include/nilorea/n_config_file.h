/**\file n_config_file.h
 *  Config file reading and writing
 *\author Castagnier Mickael
 *\version 1.0
 *\date 24/03/05
 */

#ifndef __N_CONFIG_FILE__
#define __N_CONFIG_FILE__

#ifdef __cplusplus
extern "C"
{
#endif

#include "n_hash.h"
#include "n_str.h"
#include "n_log.h"


/**\defgroup CONFIGFILE CONFIGURATIONS: classic config file loading/saving
  \addtogroup CONFIGFILE
  @{
  */

/*! maximum length of a single config line */
#define MAX_CONFIG_LINE_LEN 1024
/*! size of the hash table of config sections entries */
#define CONFIG_SECTION_HASH_TABLE_LEN 16

/*! Structure of a config section */
typedef struct CONFIG_FILE_SECTION
{
    char *section_name ;
    HASH_TABLE *entries ;
} CONFIG_FILE_SECTION ;


/*! Structure of a config file */
typedef struct CONFIG_FILE
{
    char *filename ;
    LIST *sections ;
} CONFIG_FILE ;

/* load a config from a file */
CONFIG_FILE *load_config_file( char *filename, int *errors);
/* write file from config */
int write_config_file( CONFIG_FILE *cfg_file, char *filename );
/* get a loaded config value */
char *get_config_section_value( CONFIG_FILE *cfg_file, char *section_name, int section_position, char *entry, int entry_position );
/* get the number of section for section name */
int get_nb_config_file_sections( CONFIG_FILE *cfg_file, char *section_name );
/* get the number of entries for section_name/entry */
int get_nb_config_file_sections_entries( CONFIG_FILE *cfg_file, char *section_name, int section_position, char *entry );
/* destroy a loaded config */
int destroy_config_file( CONFIG_FILE **cfg_file );

/*! Foreach elements of CONFIG_FILE macro, i.e config_foreach( config , section , key , val ); config_endfor; */
#define config_foreach( __config , __section_name , __key , __val ) \
	if( !__config || !__config -> sections ) \
	{ \
		n_log( LOG_ERR , "No config file for %s" , #__config ); \
	} \
	else  \
	{ \
		list_foreach( listnode , __config -> sections ) \
		{ \
			CONFIG_FILE_SECTION *__section = (CONFIG_FILE_SECTION *)listnode -> ptr ; \
			__section_name = __section -> section_name ; \
			ht_foreach( entry , __section -> entries ) \
			{ \
				HASH_NODE *htnode = (HASH_NODE *)entry -> ptr ; \
				if( htnode && htnode -> data . ptr ) \
				{ \
					__key = htnode -> key ; \
					__val = htnode -> data . ptr ;

/*! Foreach elements of CONFIG_FILE macro END. Will cause errors if ommitted */
#define config_endfor \
				} \
			} \
		} \
	}

/**
@}
*/

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __N_CONFIG_FILE__ */

