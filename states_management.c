/**\file states_management.c
 *
 *  level file for hacks
 *
 *\author Castagnier Micka�l aka Gull Ra Driel
 *
 *\version 1.0
 *
 *\date 29/12/2021 
 *
 */

#include "states_management.h"
#include "nilorea/n_str.h"
#include "n_fluids.h"


int load_fluid_state( N_FLUID *fluid , char *state_filename )
{
	__n_assert( state_filename , return FALSE );
	__n_assert( fluid , return FALSE );

	if( access( state_filename , F_OK ) != 0 )
	{
		n_log( LOG_INFO , "Firts run, no states to load" );
		return FALSE ;
	}

	N_STR *data = NULL ;
	data = file_to_nstr( state_filename );
	if( !data )
	{ 
		n_log( LOG_ERR , "Error reading file %s, defaults will be used" , state_filename );
		return FALSE;
	}

	cJSON *monitor_json = cJSON_Parse( _nstr( data ) );
	if (monitor_json == NULL)
	{
		const char *error_ptr = cJSON_GetErrorPtr();
		n_log( LOG_ERR , "%s: Error before: %s, defaults will be used", state_filename , _str( error_ptr ) );
		cJSON_Delete( monitor_json );
		free_nstr( &data );
		return FALSE ;
	}

	cJSON *value = NULL ;
	value = cJSON_GetObjectItemCaseSensitive( monitor_json, "numIters" );       if( cJSON_IsNumber( value ) ){ fluid -> numIters = value -> valueint ; } 
	value = cJSON_GetObjectItemCaseSensitive( monitor_json, "density" );        if( cJSON_IsNumber( value ) ){ fluid -> density  = value -> valuedouble ; } 
	value = cJSON_GetObjectItemCaseSensitive( monitor_json, "dt" );             if( cJSON_IsNumber( value ) ){ fluid -> dt       = value -> valuedouble ; } 
	value = cJSON_GetObjectItemCaseSensitive( monitor_json, "gravity" );        if( cJSON_IsNumber( value ) ){ fluid -> gravity  = value -> valuedouble ; } 
	value = cJSON_GetObjectItemCaseSensitive( monitor_json, "overRelaxation" ); if( cJSON_IsNumber( value ) ){ fluid -> overRelaxation = value -> valuedouble ; } 

	cJSON_Delete(monitor_json);
	free_nstr( &data );

	return TRUE ;
}
