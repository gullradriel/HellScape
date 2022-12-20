/**\file states_management.c
 *
 *  level file for hacks
 *
 *\author Castagnier Mickaël aka Gull Ra Driel
 *
 *\version 1.0
 *
 *\date 29/12/2021 
 *
 */

#include "states_management.h"
#include "nilorea/n_str.h"

int load_player_state( PLAYER *player , char *state_filename )
{
	__n_assert( state_filename , return FALSE );
	__n_assert( player , return FALSE );

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
	value = cJSON_GetObjectItemCaseSensitive( monitor_json, "life" );              if( cJSON_IsNumber( value ) ){ player -> attr . life = value -> valueint ; } 
	value = cJSON_GetObjectItemCaseSensitive( monitor_json, "type" );              if( cJSON_IsNumber( value ) ){ player -> attr . type = value -> valueint ; } 
	value = cJSON_GetObjectItemCaseSensitive( monitor_json, "shield" );            if( cJSON_IsNumber( value ) ){ player -> attr . shield = value -> valueint ; } 
	value = cJSON_GetObjectItemCaseSensitive( monitor_json, "xp" );                if( cJSON_IsNumber( value ) ){ player -> attr . xp = value -> valueint ; } 
	value = cJSON_GetObjectItemCaseSensitive( monitor_json, "xp_to_level" );       if( cJSON_IsNumber( value ) ){ player -> attr . xp_to_level = value -> valueint ; } 
	value = cJSON_GetObjectItemCaseSensitive( monitor_json, "level" );             if( cJSON_IsNumber( value ) ){ player -> attr . level = value -> valueint ; } 
	value = cJSON_GetObjectItemCaseSensitive( monitor_json, "current_weapon" );    if( cJSON_IsNumber( value ) ){ player -> attr . current_weapon = value -> valueint ; } 
	value = cJSON_GetObjectItemCaseSensitive( monitor_json, "have_shield" );       if( cJSON_IsNumber( value ) ){ player -> attr . have_shield = value -> valueint ; } 
	value = cJSON_GetObjectItemCaseSensitive( monitor_json, "have_sword" );        if( cJSON_IsNumber( value ) ){ player -> attr . have_sword = value -> valueint ; } 
	value = cJSON_GetObjectItemCaseSensitive( monitor_json, "have_magic_bow" );    if( cJSON_IsNumber( value ) ){ player -> attr . have_magic_bow = value -> valueint ; } 
	value = cJSON_GetObjectItemCaseSensitive( monitor_json, "have_magic_wand" );   if( cJSON_IsNumber( value ) ){ player -> attr . have_magic_wand = value -> valueint ; } 
	value = cJSON_GetObjectItemCaseSensitive( monitor_json, "nb_health_potions" ); if( cJSON_IsNumber( value ) ){ player -> attr . nb_health_potions = value -> valueint ; } 
	value = cJSON_GetObjectItemCaseSensitive( monitor_json, "nb_shield_potions" ); if( cJSON_IsNumber( value ) ){ player -> attr . nb_shield_potions = value -> valueint ; } 
	value = cJSON_GetObjectItemCaseSensitive( monitor_json, "have_key_piece_1" );  if( cJSON_IsNumber( value ) ){ player -> attr . have_key_piece_1 = value -> valueint ; } 
	value = cJSON_GetObjectItemCaseSensitive( monitor_json, "have_key_piece_2" );  if( cJSON_IsNumber( value ) ){ player -> attr . have_key_piece_2 = value -> valueint ; } 
	value = cJSON_GetObjectItemCaseSensitive( monitor_json, "have_key_piece_3" );  if( cJSON_IsNumber( value ) ){ player -> attr . have_key_piece_3 = value -> valueint ; } 
	
	value = cJSON_GetObjectItemCaseSensitive( monitor_json, "x" );  if( cJSON_IsNumber( value ) ){ player -> physics . position[ 0 ] = value -> valueint ; } 
	value = cJSON_GetObjectItemCaseSensitive( monitor_json, "y" );  if( cJSON_IsNumber( value ) ){ player -> physics . position[ 1 ] = value -> valueint ; } 
	value = cJSON_GetObjectItemCaseSensitive( monitor_json, "z" );  if( cJSON_IsNumber( value ) ){ player -> physics . position[ 2 ] = value -> valueint ; } 
	
	value = cJSON_GetObjectItemCaseSensitive( monitor_json, "level_string" );  if( cJSON_IsString( value ) ){ free_nstr( &player -> attr . level_string ); player -> attr . level_string = char_to_nstr( strdup( value -> valuestring ) ); } 

	cJSON_Delete(monitor_json);
	free_nstr( &data );

	return TRUE ;
}

int save_player_state( PLAYER *player , char *state_filename )
{
	__n_assert( player , return FALSE );
	cJSON *root = NULL ;
	root = cJSON_CreateObject();

	cJSON_AddItemToObject( root, "life", cJSON_CreateNumber( player -> attr . life ) );
	cJSON_AddItemToObject( root, "type", cJSON_CreateNumber( player -> attr . type ) );
	cJSON_AddItemToObject( root, "shield", cJSON_CreateNumber( player -> attr . shield ) );
	cJSON_AddItemToObject( root, "xp", cJSON_CreateNumber( player -> attr . xp ) );
	cJSON_AddItemToObject( root, "xp_to_level", cJSON_CreateNumber( player -> attr . xp_to_level ) );
	cJSON_AddItemToObject( root, "level", cJSON_CreateNumber( player -> attr . level ) );
	cJSON_AddItemToObject( root, "current_weapon", cJSON_CreateNumber( player -> attr . current_weapon ) );
	cJSON_AddItemToObject( root, "have_shield", cJSON_CreateNumber( player -> attr . have_shield ) );
	cJSON_AddItemToObject( root, "have_sword", cJSON_CreateNumber( player -> attr . have_sword ) );
	cJSON_AddItemToObject( root, "have_magic_bow", cJSON_CreateNumber( player -> attr . have_magic_bow ) );
	cJSON_AddItemToObject( root, "have_magic_wand", cJSON_CreateNumber( player -> attr . have_magic_wand ) );
	cJSON_AddItemToObject( root, "nb_health_potions", cJSON_CreateNumber( player -> attr . nb_health_potions ) );
	cJSON_AddItemToObject( root, "nb_shield_potions", cJSON_CreateNumber( player -> attr . nb_shield_potions ) );
	cJSON_AddItemToObject( root, "have_key_piece_1", cJSON_CreateNumber( player -> attr . have_key_piece_1 ) );
	cJSON_AddItemToObject( root, "have_key_piece_2", cJSON_CreateNumber( player -> attr . have_key_piece_2 ) );
	cJSON_AddItemToObject( root, "have_key_piece_3", cJSON_CreateNumber( player -> attr . have_key_piece_3 ) );

	cJSON_AddItemToObject( root, "x", cJSON_CreateNumber( player -> physics . position[ 0 ] ) );
	cJSON_AddItemToObject( root, "y", cJSON_CreateNumber( player -> physics . position[ 1 ] ) );
	cJSON_AddItemToObject( root, "z", cJSON_CreateNumber( player -> physics . position[ 2 ] ) );

	cJSON_AddItemToObject( root, "level_string", cJSON_CreateString( _nstr( player -> attr . level_string ) ) );

	N_STR *str_out = new_nstr( 0 );
	str_out -> data = cJSON_Print( root );
	str_out -> written = strlen( str_out -> data );
	str_out -> length = str_out -> written + 1 ;
	cJSON_Delete( root );

	nstr_to_file( str_out , state_filename );
	free_nstr( &str_out );

	return TRUE ;
}

