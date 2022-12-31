/**\file n_particles.h
 * Particles management
 *\author Castagnier Mickael aka Gull Ra Driel
 *\version 1.0
 *\date 20/12/2012
*/

#ifndef __N_PARTICLE_HEADER
#define __N_PARTICLE_HEADER

#ifdef __cplusplus
extern "C" {
#endif

/**\defgroup PARTICLES GRAPHICS: 3D particles utilities
   \addtogroup PARTICLES
  @{
*/

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>


#include "n_common.h"
#include "n_log.h"
#include "n_list.h"
#include "n_3d.h"
#include "n_time.h"

/*! classic moving particle */
#define NORMAL_PART 0
/*! sinus based moving particle */
#define SINUS_PART  1
/*! transparent particle */
#define TRANS_PART  2
/*! snow particle */
#define SNOW_PART   3
/*! fire particle */
#define FIRE_PART   4
/*! star particle */
#define STAR_PART   5
/*! circle particle */
#define CIRCLE_PART 6
/*! pixel particle */
#define PIXEL_PART  7
/*! bitmap particle */
#define BITMAP_PART 8
/*! bitmap particle */
#define TEXT_PART 9


/*! Structure of a single particle */
typedef struct PARTICLE
{
    /*! particle mode: NORMAL_PART,SINUS_PART,PIXEL_PART */
    int mode,
        /*! lifetime */
        lifetime,
        /*! sprite id in library */
        spr_id,
        /*! size of particle */
        size;

    /*! color of the particle */
    ALLEGRO_COLOR color ;

    /*! particle physical properties */
    PHYSICS object ;

} PARTICLE ;

/*! Structure of a particle system */
typedef struct PARTICLE_SYSTEM
{
    /*! list of PARTICLE pointers */
    LIST *list ;

    /*! Coordinate of emitting point */
    VECTOR3D source ;

    /*! Internal: particle system timer */
    N_TIME timer ;

    /*! Library of picture for the particles */
    ALLEGRO_BITMAP **sprites ;
    /*! size of the picture library */
    int max_sprites ;
} PARTICLE_SYSTEM ;

int init_particle_system( PARTICLE_SYSTEM **psys, int max, double x, double y, double z, int max_sprites );

int add_particle( PARTICLE_SYSTEM *psys, int spr, int mode, int lifetime, int size, ALLEGRO_COLOR color, PHYSICS object );
int add_particle_ex( PARTICLE_SYSTEM *psys, int spr, int mode, int off_x, int off_y, int lifetime, int size, ALLEGRO_COLOR color,
                     double vx, double vy, double vz,
                     double ax, double ay, double az );

int manage_particle_ex( PARTICLE_SYSTEM *psys, double delta_t );

int manage_particle( PARTICLE_SYSTEM *psys);

int draw_particle( PARTICLE_SYSTEM *psys, double xpos, double ypos, int w, int h, double range );

int free_particle_system( PARTICLE_SYSTEM **psys);

int move_particles( PARTICLE_SYSTEM *psys, double vx, double vy, double vz );

/**
@}
*/

#ifdef __cplusplus
}
#endif

#endif /* PARTICLE_HEADER_FOR_CHRISTMASHACK */

