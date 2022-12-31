/**\file n_games.h
 *  Games helpers functions
 *\author Castagnier Mickael
 *\version 1.0
 *\date 26/06/2015
 *
 */

#ifndef __N_GAMES__
#define __N_GAMES__

#ifdef __cplusplus
extern "C"
{
#endif
/**\defgroup GAME_ENV GAME APPS: game environment management utilities
   \addtogroup GAME_ENV
  @{
*/

#include "n_common.h"
#include "n_log.h"
#include "n_time.h"

/*! full use of CPU */
#define CPU_USE_FULL -1
/*! let the other process have some times */
#define CPU_USE_NICE 0
/*! wait more */
#define CPU_USE_LESS 10

/*! Game Environment structure */
typedef struct GAME_ENV
{
    /*! Store score value */
    int score,
        /*! Remainging lives */
        lifes,
        /*! wanna blur effect ? ( 0 or 1  ) */
        BLUR,
        /*! alpha value of black blit  ( 20 default ) */
        fade_value,
        /*! loop while DONE != 1 */
        DONE,
        /*! GFX_OPENGL_WINDOWED */
        GFX_CONFIG_MODE,
        /*! status of the cpu mode , CPU_USE_FULL by default */
        CPU_MODE,
        /*! time between each loop */
        loop_time,
        /*! time before updating graphics in usec*/
        draw_time,
        /*! time before a new logic update is done */
        logic_time,
        /*! time before a new logic update is done */
        nb_min_particles,
        /*! =0 for doing nothing, =1 if logic is done, = 2 if drawing is done too */
        wait_for_slowing_down_cpu,
        /*! time between two graphic frames  20000 default */
        GFX_UPDATE_RATE,
        /*! time between two logic frames  20000 default */
        LOGIC_RATE,
        /*! Measured framerate */
        real_framerate,
        /*! Level number */
        level,
        /*! position x */
        x,
        /*! position y */
        y,
        /*! position z */
        z,
        /*! left attack trigger */
        left_attack,
        /*! left attack trigger */
        right_attack,
        /*! left attack trigger */
        left_attack_pos,
        /*! left attack trigger */
        right_attack_pos ;

    /*! TIMER */
    N_TIME game_timer;

} GAME_ENV ;

/* initialize a new game environnement */
int init_game_env( GAME_ENV **game );

void destroy_game_env( GAME_ENV **game );

/* load a config from file */
GAME_ENV *load_game_config( char *config_name );

/**
@}
*/

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __N_GAMES__  */
