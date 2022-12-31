/**\file n_games.c
 *  Games helpers functions
 *\author Castagnier Mickael
 *\version 1.0
 *\date 26/06/2015
 *
 */

#include "nilorea/n_games.h"

/*!\fn init_game_env( GAME_ENV **game )
 *\param game a pointer to a game structure to allocate
 *\return TRUE or FALSE
 */
int init_game_env( GAME_ENV **game )
{
    if( !(*game) )
    {
        Malloc( (*game), GAME_ENV, 1 );
        if( !(*game ) )
        {
            n_log( LOG_ERR, "Could not allocate a new game env" );
            return FALSE ;
        }
    }
    (*game) -> BLUR = 1 ;
    (*game) -> fade_value = 20 ;
    (*game) -> DONE = 0 ;
    (*game) -> GFX_CONFIG_MODE = GFX_DIRECTX_WIN ;
    (*game) -> CPU_MODE = CPU_USE_FULL ;
    (*game) -> nb_min_particles = 0 ;
    (*game) -> loop_time = 0 ;
    (*game) -> draw_time = 0 ;
    (*game) -> logic_time = 0 ;
    (*game) -> wait_for_slowing_down_cpu = 0 ;
    (*game) -> GFX_UPDATE_RATE = 20000 ;
    (*game) -> LOGIC_RATE = 10000 ;
    (*game) -> real_framerate = 0 ;
    (*game) -> draw_time = 0 ;
    (*game) -> score = 0 ;
    (*game) -> lifes = 100 ;
    (*game) -> level = 1 ;
    (*game) -> x = 0 ;
    (*game) -> y = 0 ;
    (*game) -> z = 0 ;
    (*game) -> left_attack = 0 ;
    (*game) -> right_attack = 0 ;
    (*game) -> scrbuf = NULL ;

    return TRUE;
} /* init_game_env */



/*!\fn void destroy_game_env( GAME_ENV **game )
 *\param game a pointer a to a game structure to destroy
 */
void destroy_game_env( GAME_ENV **game )
{
    if( (*game) )
    {
        if( (*game) -> scrbuf )
            destroy_bitmap( (*game) -> scrbuf );
        Free( (*game ) );
    }
} /* destroy_game_env */



/*!\fn GAME_ENV *load_game_config( char *config_name )
 *\brief load a config from file
 *\param config_name the name of the game file config
 *\return Loaded config or NULL
 */
GAME_ENV *load_game_config( char *config_name )
{
    FILE *config = NULL ;
    char strbuf[ 1024 ] = "" ;

    /* CONFIGURATION STAGE */
    config = fopen( config_name, "rt" );

    if( !config )
    {
        n_log( LOG_ERR, "Error %s opening %s !", strerror( errno ), config_name );
        return NULL  ;
    }

    GAME_ENV *game = NULL ;
    init_game_env( &game );
    if( !game )
    {
        n_log( LOG_ERR, "Error allocating a new game object for %s !", config_name );
        goto error ;
    }

    if( fgets( strbuf, 1024, config ) == NULL )   /*error reading comment*/
    {
        n_log( LOG_ERR, "FATAL ERROR: Can not read comment ( line 1 ) in config.txt file !" );
        goto error ;
    }

    if( fgets( strbuf, 1024, config ) == NULL )
    {
        n_log( LOG_ERR, "FATAL ERROR: Can not read GFX_CONFIG_MODE in config.txt file !" );
        goto error ;
    }
    int it = 0 ;
    while(  it < 1024 && strbuf[ it ] != '\0' )
    {
        if( strbuf[ it ] == '\n' )
            strbuf[ it ] = '\0' ;
        it ++ ;
    }

    if( strcmp( strbuf, "GFX_AUTODETECT" ) == 0 )
        game -> GFX_CONFIG_MODE = GFX_AUTODETECT;
    else if( strcmp( strbuf, "GFX_AUTODETECT_WINDOWED" ) == 0 )
        game -> GFX_CONFIG_MODE = GFX_AUTODETECT_WINDOWED;
    else if( ustrcmp( strbuf, "GFX_DIRECTX_WIN" ) == 0 )
        game -> GFX_CONFIG_MODE = GFX_DIRECTX_WIN;
    else if( ustrcmp( strbuf, "GFX_DIRECTX" ) == 0 )
        game -> GFX_CONFIG_MODE = GFX_DIRECTX;
    else
    {
        n_log( LOG_NOTICE, "WARNING: NO USABLE GFX_MODE LOADED FROM CONFIG FILE ! USING DEFAULT GFX_DIRECTX_WIN! tmpstr:\"%s\"", strbuf );
        game -> GFX_CONFIG_MODE = GFX_DIRECTX_WIN;
    }

    if( fgets( strbuf, 1024, config ) == NULL )   /*error reading comment*/
    {
        n_log( LOG_ERR, "FATAL ERROR: Can not read comment ( line 3 ) in config.txt file !" );
        goto error ;
    }

    if( fgets( strbuf, 1024, config ) == NULL )
    {
        n_log( LOG_ERR, "FATAL ERROR: Can not read CPU_MODE in config.txt file ! \n");
        goto error ;
    }

    if( strncmp( strbuf, "CPU_USE_FULL", 12 ) == 0 )
        game -> CPU_MODE = CPU_USE_FULL;
    else if( strncmp( strbuf, "CPU_USE_NICE", 12 ) == 0 )
        game -> CPU_MODE = CPU_USE_NICE;
    else if( strncmp( strbuf, "CPU_USE_LESS", 12 ) == 0 )
        game -> CPU_MODE = CPU_USE_LESS;
    else
        n_log( LOG_NOTICE,  "WARNING: NO USABLE CPU_MODE LOADED FROM CONFIG FILE ! USING DEFAULT CPU_FULL_USE !\n" );


    if( fgets( strbuf, 1024, config ) == NULL )   /*error reading comment*/
    {
        n_log( LOG_ERR, "FATAL ERROR: Can not read comment ( line 7 ) in config.txt file !" );
        goto error ;
    }

    if( fgets( strbuf, 1024, config ) == NULL )
    {
        n_log( LOG_ERR, "FATAL ERROR: Can not read nb_min_particles in config.txt file !");
        goto error ;
    }
    game -> nb_min_particles = strtol( strbuf, NULL, 10 );
    if( game -> nb_min_particles < 10 )
        game -> nb_min_particles = 10 ;

    if( fgets( strbuf, 1024, config ) == NULL )   /*error reading comment*/
    {
        n_log( LOG_ERR, "FATAL ERROR: Can not read comment ( line 9 ) in config.txt file !" );
        goto error ;
    }

    if( fgets( strbuf, 1024, config ) == NULL )
    {
        n_log( LOG_ERR, "FATAL ERROR: Can not read DRAWING_UPDATE_RATE in config.txt file !");
        goto error ;
    }

    game -> GFX_UPDATE_RATE = strtol( strbuf, NULL, 10 );

    if( game -> GFX_UPDATE_RATE < 0 )
    {
        n_log( LOG_NOTICE, "WARNING: You can not have a negative or zero GFX_UPDATE_RATE\nDefault value ( 20000 ) will be used" );
        game -> GFX_UPDATE_RATE = 20000;
    }
    else if( game -> GFX_UPDATE_RATE > 1000000 )
    {
        n_log( LOG_NOTICE, "WARNING: You would not want to have a 1 second GFX_UPDATE_RATE, no ?\nDefault value ( 20000 ) will be used" );
        game -> GFX_UPDATE_RATE = 20000;
    }

    if( fgets( strbuf, 1024, config ) == NULL )   /*error reading comment*/
    {
        n_log( LOG_ERR, "FATAL ERROR: Can not read comment ( line 11 ) in config.txt file ! \n");
        goto error ;
    }

    if( fgets( strbuf, 1024, config ) == NULL )
    {
        n_log( LOG_ERR, "FATAL ERROR: Can not read LOGIC_RATE in config.txt file !");
        goto error ;
    }

    game -> LOGIC_RATE = strtol( strbuf, NULL, 10 );

    if( game -> LOGIC_RATE < 0 )
    {
        n_log( LOG_NOTICE, "WARNING: You can not have a negative or zero LOGIC_RATE\nDefault value ( 10000 ) will be used" );
        game -> LOGIC_RATE = 10000;
    }
    else
    {
        if( game -> LOGIC_RATE > 1000000 )
        {
            n_log( LOG_NOTICE, "WARNING: You would not want to have a 1 second LOGIC_RATE, no ?\nDefault value ( 10000 ) will be used" );
            game -> LOGIC_RATE = 20000;
        }
    }
    fclose( config );
    goto clean ;

error:
    destroy_game_env( &game );

clean:

    return game ;
} /* destroy_game_env */
