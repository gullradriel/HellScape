/**\file states_management.h
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



#ifndef STATES_HEADER_FOR_HACKS
#define STATES_HEADER_FOR_HACKS

#ifdef __cplusplus
extern "C" {
#endif

#include "level.h"
#include "cJSON.h"

	int load_player_state( PLAYER *player , char *state_filename );
	int save_player_state( PLAYER *player , char *state_file );

#ifdef __cplusplus
}
#endif

#endif
