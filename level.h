/**\file level.h
 *
 *  level file for hacks
 *
 *\author Castagnier Mickaël aka Gull Ra Driel
 *
 *\version 1.0
 *
 *\date 29/12/2021 based on 30/12/2016
 *
 */



#ifndef LEVEL_HEADER_FOR_HACKS
#define LEVEL_HEADER_FOR_HACKS

#ifdef __cplusplus
extern "C" {
#endif

#include "nilorea/n_common.h"
#include "nilorea/n_log.h"
#include "nilorea/n_str.h"
#include "nilorea/n_hash.h"
#include "nilorea/n_anim.h"
#include "nilorea/n_particles.h"
#include "allegro5/allegro_audio.h"
#include "allegro5/allegro_acodec.h"

#define WIDTH 1280
#define HEIGHT 800

#define RESERVED_SAMPLES   16
#define MAX_SAMPLE_DATA    10

#define ACTION_MOVE 2
#define ACTION_ATTACK 4

#define PASSIVE_MONSTER 10
#define AGRESSIVE_MONSTER 15
#define BIGBOSS_MONSTER 25
#define FINAL_MONSTER 35

#define EMPTY_CELL     0
#define BLOCKING_CELL  1
#define PASSING_CELL_1 2
#define PASSING_CELL_2 3
#define EXIT_MAIN_CELL 4
#define EXIT_D1_CELL   5
#define EXIT_D2_CELL   6
#define EXIT_D3_CELL   7
#define EXIT_D4_CELL   8
#define SWORD_CELL     9
#define BOW_CELL       10
#define WAND_CELL      11
#define SHIELD_CELL    12
#define KEY_FRAG1      13
#define KEY_FRAG2      14
#define KEY_FRAG3      15
#define ARROW          16

	typedef struct ATTRIBUTES
	{
		int	life,
			shield,
			type,
			action,
			move,
			xp,
			xp_to_level,
			level,
			direction,
			xspeedmax,
			yspeedmax,
			xspeedinc,
			yspeedinc,
			next_move_timer,
			current_weapon,
			have_shield,
			have_sword,
			have_magic_bow,
			have_magic_wand,
			nb_health_potions,
			nb_shield_potions,
			have_key_piece_1,
			have_key_piece_2,
			have_key_piece_3,
			have_finished;
		N_STR *level_string;
	} ATTRIBUTES ;

	typedef struct MONSTER
	{
		/*! gfx */
		ANIM_DATA anim ;

		/* physic properties */
		PHYSICS	physics ;
		int action ;

		/* characteristics */
		ATTRIBUTES attr ;
	} MONSTER ;

	typedef struct PLAYER
	{
		PHYSICS physics ;
		ANIM_DATA anim ;
		ATTRIBUTES attr ;
	} PLAYER;

	MONSTER *new_monster( int life, int type, PHYSICS physics );

	typedef struct LEVEL
	{

		int **cells,
		    w,
		    h,
		    tilew,
		    tileh,
		    native_w,
		    native_h,
		    exitx,
		    exity,
		    m1,
		    m2,
		    m3;

		double startx, starty ;

		LIST *monster_list ;

		ALLEGRO_BITMAP *tiles[ 32 ];

		PARTICLE_SYSTEM *particle_system_effects ;
		PARTICLE_SYSTEM *particle_system_bullets ;

		VECTOR3D friction ;
		VECTOR3D gravity ;

	} LEVEL ;

	void set_gui_message( char *message );
	int get_level_data( LEVEL *level, PHYSICS *physics, int mx, int my, int *x, int *y );
	LEVEL *load_level( PLAYER *player, ANIM_LIB *lib , int w, int h );
	int draw_level( LEVEL *lvl, PLAYER *player , int w, int h);
	int test_coord( LEVEL *level, PHYSICS *physics, VECTOR3D fricton, int off_x, int off_y );
	int animate_physics( LEVEL *level, PHYSICS *physics, VECTOR3D friction , double delta_t );
	int animate_level( LEVEL *level, PLAYER *player, double delta_t );

#ifdef __cplusplus
}
#endif
#endif
