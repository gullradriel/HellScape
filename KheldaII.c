/**\file KheldaII.c
 *
 *  KheldaII Main File
 *
 *\author Castagnier Mickaël
 *
 *\version 1.0
 *
 *\date 30/12/2021
 */


#include "nilorea/n_common.h"
#include "nilorea/n_particles.h"
#include "nilorea/n_anim.h"
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_ttf.h>
#include "level.h"
#include "states_management.h"
#include "locale.h"

ALLEGRO_DISPLAY *display  = NULL ;

/******************************************************************************
 *                           VARIOUS DECLARATIONS                             *
 ******************************************************************************/

int		DONE = 0,                    /* Flag to check if we are always running */
		getoptret = 0,				  /* launch parameter check */
		log_level = LOG_ERR ;			 /* default LOG_LEVEL */

ALLEGRO_BITMAP *wood_stick_bmp = NULL ;
ALLEGRO_BITMAP *sword_bmp      = NULL ;


ALLEGRO_TIMER *fps_timer = NULL ;
ALLEGRO_TIMER *logic_timer = NULL ;
LIST *active_object = NULL ;                      /* list of active objects */

LEVEL *level = NULL ;

int main( int argc, char *argv[] )
{

	/* Set the locale to the POSIX C environment */
	setlocale (LC_ALL, "POSIX");

	/*
	 * INITIALISATION
	 */
	set_log_level( LOG_NOTICE );

	N_STR *log_file = NULL ;
	nstrprintf( log_file, "%s.log", argv[ 0 ] ) ;
	/*set_log_file( _nstr( log_file ) );*/
	free_nstr( &log_file );

	n_log( LOG_NOTICE, "%s is starting ...", argv[ 0 ] );

	/* allegro 5 + addons loading */
	if (!al_init())
	{
		n_abort("Could not init Allegro.\n");
	}
	if( !al_init_acodec_addon() )
	{
		n_abort("Could not register addons.\n");
	}
	if (!al_install_audio())
	{
		n_abort("Unable to initialize audio addon\n");
	}
	if (!al_init_acodec_addon())
	{
		n_abort("Unable to initialize acoded addon\n");
	}
	if (!al_init_image_addon())
	{
		n_abort("Unable to initialize image addon\n");
	}
	if (!al_init_primitives_addon() )
	{
		n_abort("Unable to initialize primitives addon\n");
	}
	if( !al_init_font_addon() )
	{
		n_abort("Unable to initialize font addon\n");
	}
	if( !al_init_ttf_addon() )
	{
		n_abort("Unable to initialize ttf_font addon\n");
	}
	if( !al_install_keyboard() )
	{
		n_abort("Unable to initialize keyboard handler\n");
	}
	if( !al_install_mouse())
	{
		n_abort("Unable to initialize mouse handler\n");
	}

	if( !al_reserve_samples( RESERVED_SAMPLES ) ) 
	{
		n_abort("Could not set up voice and mixer.\n");
	}

	ALLEGRO_SAMPLE *sample_data[MAX_SAMPLE_DATA] = {NULL};
	memset(sample_data, 0, sizeof(sample_data));


	ALLEGRO_EVENT_QUEUE *event_queue = NULL;

	event_queue = al_create_event_queue();
	if(!event_queue)
	{
		fprintf(stderr, "failed to create event_queue!\n");
		al_destroy_display(display);
		return -1;
	}
	char ver_str[ 128 ] = "" ;

	while( ( getoptret = getopt( argc, argv, "hvV:L:" ) ) != EOF )
	{
		switch( getoptret )
		{
			case 'h':
				n_log( LOG_NOTICE, "\n    %s -h help -v version -V DEBUGLEVEL (NOLOG,VERBOSE,NOTICE,ERROR,DEBUG)\n", argv[ 0 ] );
				exit( TRUE );
			case 'v':
				sprintf( ver_str, "%s %s", __DATE__, __TIME__ );
				exit( TRUE );
				break ;
			case 'V':
				if( !strncmp( "INFO", optarg, 6 ) )
				{
					log_level = LOG_INFO ;
				}
				else
				{	if( !strncmp( "NOTICE", optarg, 6 ) )
					{
						log_level = LOG_NOTICE ;				}
					else
					{
						if(  !strncmp( "VERBOSE", optarg, 7 ) )
						{
							log_level = LOG_NOTICE ;
						}
						else
						{
							if(  !strncmp( "ERROR", optarg, 5 ) )
							{
								log_level = LOG_ERR ;
							}
							else
							{
								if(  !strncmp( "DEBUG", optarg, 5 ) )
								{
									log_level = LOG_DEBUG ;
								}
								else
								{
									n_log( LOG_ERR, "%s is not a valid log level", optarg );
									exit( FALSE );
								}
							}
						}
					}
				}
				n_log( LOG_NOTICE, "LOG LEVEL UP TO: %d", log_level );
				set_log_level( log_level );
				break;
			case 'L' :
				n_log( LOG_NOTICE, "LOG FILE: %s", optarg );
				set_log_file( optarg );
				break ;
			case '?' :
				{
					switch( optopt )
					{
						case 'V' :
							n_log( LOG_ERR, "\nPlease specify a log level after -V. \nAvailable values: NOLOG,VERBOSE,NOTICE,ERROR,DEBUG" );
							break;
						case 'L' :
							n_log( LOG_ERR, "\nPlease specify a log file after -L" );
						default:
							break;
					}
				}
				__attribute__ ((fallthrough));
			default:
				n_log( LOG_ERR, "\n    %s -h help -v version -V DEBUGLEVEL (NOLOG,VERBOSE,NOTICE,ERROR,DEBUG) -L logfile", argv[ 0 ] );
				exit( FALSE );
		}
	}


	al_set_new_display_flags( ALLEGRO_OPENGL|ALLEGRO_WINDOWED );
	display = al_create_display( WIDTH, HEIGHT );
	if( !display )
	{
		n_abort("Unable to create display\n");
	}
	al_set_window_title( display, "KheldaII" );

	al_set_new_bitmap_flags( ALLEGRO_VIDEO_BITMAP );

	ALLEGRO_FONT *font = al_load_font( "DATA/2Dumb.ttf", 18, 0 );

	DONE = 0 ;
	double fps = 50.0 ;
	fps_timer = al_create_timer( 1.0 / fps );
	double logictime = fps * 2 ;
	logic_timer = al_create_timer( 1.0 / logictime );
	double delta_time = 1000000.0 / logictime ;


	PLAYER player ;
	memset( &player . attr , 0 , sizeof( ATTRIBUTES ) );
	nstrprintf( player . attr . level_string , "DATA/Levels/main.txt" );
	player . attr . level = 1 ;

	al_register_event_source(event_queue, al_get_display_event_source(display));
	al_start_timer( fps_timer );
	al_start_timer( logic_timer );
	al_register_event_source(event_queue, al_get_timer_event_source(fps_timer));
	al_register_event_source(event_queue, al_get_timer_event_source(logic_timer));

	al_register_event_source(event_queue, al_get_keyboard_event_source());
	al_register_event_source(event_queue, al_get_mouse_event_source());

	ALLEGRO_BITMAP *scrbuf = al_create_bitmap( WIDTH, HEIGHT );
	__n_assert( ( wood_stick_bmp = al_load_bitmap( "DATA/Gfxs/fallersword_wood.png" ) ) , n_log( LOG_ERR , "load bitmap DATA/Gfxs/fallersword_wood.png returned null" ); exit( 1 ); );
	__n_assert( ( sword_bmp      = al_load_bitmap( "DATA/Gfxs/fallersword.png"  ) )     , n_log( LOG_ERR , "load bitmap DATA/Gfxs/fallersword.png returned null" ); exit( 1 ); );

	al_hide_mouse_cursor(display);

	/* Main loop */

	/* Start on main map for first load */
	player . attr . life = 100 ;
	player . physics . sz = 8 ;
	player . attr . xp = 0 ;
	player . attr . xp_to_level = 1000 ;


	enum APP_KEYS
	{
		KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_ESC, KEY_SPACE, KEY_CTRL , KEY_SHIFT , KEY_PAD_MINUS , KEY_PAD_PLUS , KEY_PAD_ENTER , KEY_M , KEY_W , KEY_F1 , KEY_F2 , KEY_F3 , KEY_F4
	};
	int key[ 17 ] = {false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false};

	int intro_message_delay = 3000000 ; //3 secs
	double weapon_current_angle = -1 ;
	double weapon_start_angle = -1 ;
	double weapon_end_angle = -1 ;

	load_player_state( &player , "player_state.json" );

	int gui_message_delay = 0 , gui_message_x = WIDTH / 2 , gui_message_y = HEIGHT / 2 ;
	char *gui_message = NULL ;

	void set_gui_message( char *message )
	{
		gui_message_delay = 1500000 ;
		FreeNoLog( gui_message );
		gui_message = message ;
		gui_message_y = HEIGHT / 2 ;
	}

#define BAT_ANIM 0
#define DRAGONFLY_ANIM 1

	ANIM_LIB *animations = create_anim_library( "Animations" , 100 );
	add_bmp_to_lib( animations , 0 , "DATA/Gfxs/chauve_souris_bas.png", "DATA/Gfxs/chauve_souris_bas.txt" );
	add_bmp_to_lib( animations , 1 , "DATA/Gfxs/libellule_bas.png", "DATA/Gfxs/libellule_bas.txt" );
	add_bmp_to_lib( animations , 2 , "DATA/Gfxs/plante.png", "DATA/Gfxs/plante.txt" );
	add_bmp_to_lib( animations , 3 , "DATA/Gfxs/player_haut.png", "DATA/Gfxs/player_haut.txt" );
	add_bmp_to_lib( animations , 4 , "DATA/Gfxs/player_haut_droit.png", "DATA/Gfxs/player_haut_droit.txt" );
	add_bmp_to_lib( animations , 5 , "DATA/Gfxs/player_droit.png", "DATA/Gfxs/player_droit.txt" );
	add_bmp_to_lib( animations , 6 , "DATA/Gfxs/player_bas_droit.png", "DATA/Gfxs/player_bas_droit.txt" );
	add_bmp_to_lib( animations , 7 , "DATA/Gfxs/player_bas.png", "DATA/Gfxs/player_bas.txt" );
	add_bmp_to_lib( animations , 8 , "DATA/Gfxs/player_bas_gauche.png", "DATA/Gfxs/player_bas_gauche.txt" );
	add_bmp_to_lib( animations , 9 , "DATA/Gfxs/player_gauche.png", "DATA/Gfxs/player_gauche.txt" );
	add_bmp_to_lib( animations , 10 , "DATA/Gfxs/player_haut_gauche.png", "DATA/Gfxs/player_haut_gauche.txt" );
	add_bmp_to_lib( animations , 11 , "DATA/Gfxs/ogre_droit.png", "DATA/Gfxs/ogre_droit.txt" );
	add_bmp_to_lib( animations , 12 , "DATA/Gfxs/ogre_gauche.png", "DATA/Gfxs/ogre_gauche.txt" );

	if( !( sample_data[ 0 ]  = al_load_sample( "DATA/Musics/background_music.ogg" ) ) )
	{
		n_abort( "Could not load DATA/Musics/background_music.ogg" );
	}
	al_play_sample(sample_data[ 0 ] , 1 , 0 , 1 , ALLEGRO_PLAYMODE_LOOP , NULL );

	do
	{
		memset( &key , 0 , 8 * sizeof( int ) );
		active_object = new_generic_list( -1 );
		LEVEL *level = NULL ;
		level = load_level( &player , animations , WIDTH, HEIGHT );
		if( level -> gravity[ 1 ] > 0.0 )
		{
			player . anim . id = 5 ;
		}
		else
		{
			//player . anim . id = 7 ;
			player . anim . id = 5 ;
		}
		player . anim . lib = animations ;
		player . anim . frame = rand()%animations -> gfxs[ player . anim . id ] -> nb_frames ;
		VECTOR3D_SET( player . physics . speed,  0.0, 0.0, 0.0  );
		VECTOR3D_SET( player . physics . acceleration, 0.0, 0.0, 0.0 );
		VECTOR3D_SET( player . physics . orientation, 0.0, 0.0, 0.0 );
		VECTOR3D_SET( player . physics . position, level -> startx * level -> tilew ,  level -> starty * level -> tileh , 0.0 );
		player . physics . can_jump = 0, player . physics . type = 0 ;
		player . attr . action = 0 ;
		player . attr . move = 0 ;
		player . attr . direction = 0 ;
		player . attr . xspeedmax = 150 ;
		player . attr . xspeedinc = 10 ;
		if( level -> gravity[ 1 ] != 0.0 )
		{
			player . attr . yspeedmax = 250 ;
			player . attr . yspeedinc = 250 ;
		}
		else
		{
			player . attr . yspeedmax = 100 ;
			player . attr . yspeedinc = 10 ;
		}

		load_player_state( &player , "player_state.json" );

		int nb_health_potions = player .attr . nb_health_potions ;
		int nb_shield_potions = player .attr . nb_shield_potions ;
		int player_xp = player . attr . xp ;
		int potions_delay = 0 ;

		if( DONE > 0 )
		{
			VECTOR3D_SET( player . physics . position, level -> startx * level -> tilew ,  level -> starty * level -> tileh , 0.0 );
			DONE = 0 ;
		}

		VECTOR3D_SET( player . physics . gravity , level -> gravity[ 0 ] , level -> gravity[ 1 ] , level -> gravity[ 2 ] );

		double shoot_rate_time = 0 ;

		DONE = 0 ;

		VECTOR3D old_pos ;

		int mx = 0, my = 0, mouse_b1 = 0, mouse_b2 = 0 ;
		int do_draw = 0, do_logic = 0 ;
		int mouse_button = 0 ;
		al_flush_event_queue( event_queue );
		do
		{
			do
			{
				ALLEGRO_EVENT ev ;

			al_wait_for_event(event_queue, &ev);

			if(ev.type == ALLEGRO_EVENT_KEY_DOWN)
			{
				switch(ev.keyboard.keycode)
				{
					case ALLEGRO_KEY_UP:
						key[KEY_UP] = 1;
						break;
					case ALLEGRO_KEY_DOWN:
						key[KEY_DOWN] = 1;
						break;
					case ALLEGRO_KEY_LEFT:
						key[KEY_LEFT] = 1;
						break;
					case ALLEGRO_KEY_RIGHT:
						key[KEY_RIGHT] = 1;
						break;
					case ALLEGRO_KEY_ESCAPE:
						key[KEY_ESC] = 1 ;
						break;
					case ALLEGRO_KEY_SPACE:
						key[KEY_SPACE] = 1 ;
						break;
					case ALLEGRO_KEY_LSHIFT:
					case ALLEGRO_KEY_RSHIFT:
						key[KEY_SHIFT] = 1 ;
						break;
					case ALLEGRO_KEY_PAD_MINUS:
						key[KEY_PAD_MINUS] = 1 ;
						break;
					case ALLEGRO_KEY_PAD_PLUS:
						key[KEY_PAD_PLUS] = 1 ;
						break;
					case ALLEGRO_KEY_PAD_ENTER:
						key[KEY_PAD_ENTER] = 1 ;
						break;
					case ALLEGRO_KEY_M:
						key[KEY_M] = 1 ;
						break;
					case ALLEGRO_KEY_W:
						key[KEY_W] = 1 ;
						break;
					case ALLEGRO_KEY_LCTRL:
					case ALLEGRO_KEY_RCTRL:
						key[KEY_CTRL] = 1 ;
						break;
					case ALLEGRO_KEY_F1:
						key[ KEY_F1 ] = 1 ;
						break;
					case ALLEGRO_KEY_F2:
						key[ KEY_F2 ] = 1 ;
						break;
					case ALLEGRO_KEY_F3:
						key[ KEY_F3 ] = 1 ;
						break;
					case ALLEGRO_KEY_F4:
						key[ KEY_F4 ] = 1 ;
						break;

					default:
						break;
				}
			}
			else if(ev.type == ALLEGRO_EVENT_KEY_UP)
			{
				switch(ev.keyboard.keycode)
				{
					case ALLEGRO_KEY_UP:
						key[KEY_UP] = 0;
						break;
					case ALLEGRO_KEY_DOWN:
						key[KEY_DOWN] = 0;
						break;
					case ALLEGRO_KEY_LEFT:
						key[KEY_LEFT] = 0;
						break;
					case ALLEGRO_KEY_RIGHT:
						key[KEY_RIGHT] =0;
						break;
					case ALLEGRO_KEY_ESCAPE:
						key[KEY_ESC] = 0 ;
						break;
					case ALLEGRO_KEY_SPACE:
						key[KEY_SPACE] = 0 ;
						break;
					case ALLEGRO_KEY_LSHIFT:
					case ALLEGRO_KEY_RSHIFT:
						key[KEY_SHIFT] = 0 ;
						break;
					case ALLEGRO_KEY_PAD_MINUS:
						key[KEY_PAD_MINUS] = 0 ;
						break;
					case ALLEGRO_KEY_PAD_PLUS:
						key[KEY_PAD_PLUS] = 0 ;
						break;
					case ALLEGRO_KEY_PAD_ENTER:
						key[KEY_PAD_ENTER] = 0 ;
						break;
					case ALLEGRO_KEY_M:
						key[KEY_M] = 0 ;
						break;
					case ALLEGRO_KEY_W:
						key[KEY_W] = 0 ;
						break;
					case ALLEGRO_KEY_LCTRL:
					case ALLEGRO_KEY_RCTRL:
						key[KEY_CTRL] = 0 ;
						break;
					case ALLEGRO_KEY_F1:
						key[ KEY_F1 ] = 0 ;
						break;
					case ALLEGRO_KEY_F2:
						key[ KEY_F2 ] = 0 ;
						break;
					case ALLEGRO_KEY_F3:
						key[ KEY_F3 ] = 0 ;
						break;
					case ALLEGRO_KEY_F4:
						key[ KEY_F4 ] = 0 ;
						break;
					default:
						break;
				}
			}
			else if( ev.type == ALLEGRO_EVENT_TIMER )
			{
				if( al_get_timer_event_source( fps_timer ) == ev.any.source )
				{
					do_draw = 1 ;
				}
				else if( al_get_timer_event_source( logic_timer ) == ev.any.source )
				{
					do_logic = 1;
				}
			}
			else if( ev.type == ALLEGRO_EVENT_MOUSE_AXES )
			{
				mx = ev.mouse.x;
				my = ev.mouse.y;
			}
			else if( ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN )
			{
				if( ev.mouse.button == 1 )
					mouse_b1 = 1 ;
				if( ev.mouse.button == 2 )
					mouse_b2 = 1 ;
			}
			else if( ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP )
			{
				if( ev.mouse.button == 1 )
					mouse_b1 = 0 ;
				if( ev.mouse.button == 2 )
					mouse_b2 = 0 ;
			}

			/* Processing inputs */
			mouse_button = -1 ;
			if( mouse_b1==1 )
				mouse_button = 1 ;
			if( mouse_b2==1 )
				mouse_button = 2 ;
			else if( ev.type == ALLEGRO_EVENT_DISPLAY_SWITCH_IN || ev.type == ALLEGRO_EVENT_DISPLAY_SWITCH_OUT )
            {
                al_clear_keyboard_state( display );
                al_flush_event_queue( event_queue );
            }
            /*else
            {
                // Processing inputs 
                get_keyboard( chat_line , ev );
            } */
        	}while( !al_is_event_queue_empty( event_queue) );
		
			if( do_logic == 1 )
			{
				player . attr . move = 0 ;
				if( key[ KEY_UP ] )
				{
					player . attr . move = 1 ;
					if( ( ( player . physics . can_jump != 0 ) || player . physics . gravity[ 1 ] == 0.0 ) )
					{
						VECTOR3D_SET( player . physics . speed, player . physics . speed[ 0 ], player . physics . speed[ 1 ] - player . attr . yspeedinc, player . physics . speed[ 2 ] );
						if( player . physics . speed[ 1 ] < -player . attr . yspeedmax )
							player . physics . speed[ 1 ] = -player . attr . yspeedmax ;

						player . physics . can_jump = 0 ;
						player . attr . move = 1 ;
					}
				}
				else
				{
					if( player . physics . speed[ 1 ] < 0.0 )
					{
						player . physics . speed[ 1 ] = player . physics . speed[ 1 ] + level -> friction[ 1 ] * delta_time / 1000000.0 ;
						if( player . physics . speed[ 1 ] > 0.0 )
							player . physics . speed[ 1 ] = 0.0 ;
					}
				}


				if( key[ KEY_DOWN ] )
				{
					VECTOR3D_SET( player . physics . speed, player . physics . speed[ 0 ], player . physics . speed[ 1 ] + player . attr . yspeedinc, player . physics . speed[ 2 ] );
					if( player . physics . speed[ 1 ] > player . attr . yspeedmax )
						player . physics . speed[ 1 ] = player . attr . yspeedmax ;
					player . physics . can_jump = 0 ;
					player . attr . move = 1 ;
				}
				else
				{
					if( player . physics . speed[ 1 ] > 0.0 )
					{
						player . physics . speed[ 1 ] = player . physics . speed[ 1 ] - level -> friction[ 1 ] * delta_time / 1000000.0 ;
						if( player . physics . speed[ 1 ] < 0.0 )
							player . physics . speed[ 1 ] = 0.0 ;
					}
				}

				if( key[ KEY_LEFT ] )
				{
					VECTOR3D_SET( player . physics . speed, player . physics . speed[ 0 ] - player . attr . xspeedinc, player . physics . speed[ 1 ], player . physics . speed[ 2 ] );
					if( player . physics . speed[ 0 ] < -player . attr . xspeedmax )
						player . physics . speed[ 0 ] = -player . attr . xspeedmax ;
					player . attr . direction = 1 ;
					player . attr . move = 1 ;
				}
				else
				{
					if( player . physics . speed[ 0 ] < 0.0 )
					{
						player . physics . speed[ 0 ] = player . physics . speed[ 0 ] + level -> friction[ 0 ] * delta_time / 1000000.0 ;
						if( player . physics . speed[ 0 ] > 0.0 )
							player . physics . speed[ 0 ] = 0.0 ;
					}
				}


				if( key[ KEY_RIGHT ] )
				{
					VECTOR3D_SET( player . physics . speed, player . physics . speed[ 0 ] + player . attr . xspeedinc, player . physics . speed[ 1 ], player . physics . speed[ 2 ] );
					if( player . physics . speed[ 0 ] > player . attr . xspeedmax )
						player . physics . speed[ 0 ] = player . attr . xspeedmax ;
					player . attr . direction = 1 ;
					player . attr . move = 1 ;
				}
				else
				{
					if( player . physics . speed[ 0 ] > 0.0 )
					{
						player . physics . speed[ 0 ] = player . physics . speed[ 0 ] - level -> friction[ 0 ] * delta_time / 1000000.0 ;
						if( player . physics . speed[ 0 ] < 0.0 )
							player . physics . speed[ 0 ] = 0.0 ;
					}
				}

				if( key[ KEY_F1 ] )
				{
					player . attr . current_weapon = 0 ;
				}
				if( key[ KEY_F2 ] && player . attr . have_sword )
				{
					player . attr . current_weapon = 1 ;
				}
				if( key[ KEY_F3 ] && player . attr . have_magic_bow )
				{
					player . attr . current_weapon = 2 ;
				}
				if( key[ KEY_F4 ] && player . attr . have_magic_wand )
				{
					player . attr . current_weapon = 3 ;
				}

				if( key[KEY_CTRL ]  || mouse_button == 1 )
				{
					if( player . attr . action == 0 )
						player . attr . action = 2 ;

					if( player . attr . action == 3 )
					{
						if( shoot_rate_time > ( ( 11 - player . attr . level ) * 53333 ) )
						{
							player . attr . action = 2 ;
							shoot_rate_time -= ( ( 11 - player . attr . level ) * 53333 )  ;
						}
					}
				}
				else
				{
					shoot_rate_time = 0 ;
					player . attr . action = 0 ;
				}


				if( key[ KEY_PAD_ENTER ] )
				{
					if( potions_delay == 0 )
					{
						potions_delay += 500000;
						if( player . attr . nb_health_potions > 0 )
						{
							player . attr . nb_health_potions -- ;
							player . attr . life += 30 ;
							set_gui_message( strdup( "+30 hp" ) );
							save_player_state( &player , "player_state.json" );
						}
						else
							set_gui_message( strdup( "!! no Health potions !!" ) );

					}
				}
				if( key[ KEY_PAD_PLUS ] )
				{
					if( potions_delay == 0 )
					{
						potions_delay += 500000 ;
						if( player . attr . nb_shield_potions > 0 )
						{
							player . attr . nb_shield_potions -- ;
							player . attr . shield += 30 ;
							set_gui_message( strdup( "+30 sp" ) );
							save_player_state( &player , "player_state.json" );
						}
						else
							set_gui_message( strdup( "!! no Shield potions !!" ) );
					}
				}

				if( potions_delay > 0 )
				{
					potions_delay -= delta_time ;
					if( potions_delay < 0 )
						potions_delay = 0 ;
				}

				shoot_rate_time += delta_time ;

				if( fabs( player . physics . speed[ 0 ] ) < 0.5 )
					player . physics . speed[ 0 ] = 0.0 ;

				if( level -> gravity[ 1 ] > 0.0 )
				{
					if( player . physics . speed[ 0 ] < 0.0 )
						player . anim . id = 9 ;
					if( player . physics . speed[ 0 ] > 0.0 )
						player . anim . id = 5 ;
				}
				else
				{
					if( player . physics. speed[ 0 ] == 0.0 && player . physics . speed[ 1 ] < 0.0 )
					{
						player . anim . id = 3 ;
					}
					if( player . physics . speed[ 0 ] > 0.0 && player . physics . speed[ 1 ] < 0.0 )
					{
						player . anim . id = 4 ;
					}
					if( player . physics . speed[ 0 ] > 0.0 && player . physics . speed[ 1 ] == 0.0 )
					{
						player . anim . id = 5 ;
					}
					if( player . physics . speed[ 0 ] > 0.0 && player . physics . speed[ 1 ] > 0.0 )
					{
						player . anim . id = 6 ;
					}
					if( player . physics . speed[ 0 ] == 0.0 && player . physics . speed[ 1 ] > 0.0 )
					{
						player . anim . id = 7 ;
					}
					if( player . physics . speed[ 0 ] < 0.0 && player . physics . speed[ 1 ] > 0.0 )
					{
						player . anim . id = 8 ;
					}
					if( player . physics . speed[ 0 ] < 0.0 && player . physics . speed[ 1 ] == 0.0 )
					{
						player . anim . id = 9 ;
					}
					if( player . physics . speed[ 0 ] < 0.0 && player . physics . speed[ 1 ] < 0.0 )
					{
						player . anim . id = 10 ;
					}
				}

				if( player . attr . move == 1 || fabs( player . physics . speed[ 0 ] ) > 0.0 || ( level -> gravity[ 1 ] == 0.0 && fabs( player . physics . speed[ 1 ] ) > 0.0 ) )
					update_anim( &player . anim , delta_time );

				memcpy( &old_pos, &player . physics . position, sizeof( VECTOR3D ) );

				animate_physics( level, &player . physics, level -> friction, delta_time );


				if( player . attr . have_finished == 0 )
				{
					animate_level( level, &player, delta_time );
				}
				else
				{
					if( rand()%100 > 15 )
					{
						int vx = WIDTH/2 - rand()%WIDTH ;
						int vy = HEIGHT/2 - rand()%WIDTH ;

						for( int it = 10 ; it < 50 + rand()%150 ; it ++ )
						{	PHYSICS tmp_part ;
							tmp_part . sz = rand()%10  ;
							tmp_part . type = 100 ;
							VECTOR3D_SET( tmp_part . speed, 200 - rand()%400, 200 - rand()%400, 0.0  );
							VECTOR3D_SET( tmp_part . position, player . physics . position[ 0 ] + vx , player . physics . position[ 1 ] + vy , 0.0  );
							VECTOR3D_SET( tmp_part . acceleration, 0.0, 0.0, 0.0 );
							VECTOR3D_SET( tmp_part . orientation, 0.0, 0.0, 0.0 );
							add_particle( level -> particle_system_effects, -1, PIXEL_PART, rand()%500, 1+rand()%2, al_map_rgba( rand()%255 , rand()%255 , rand()%255, rand()%255 ), tmp_part );
						}
						LIST_NODE *node = NULL ;
						PARTICLE *ptr = NULL ;
						node = level -> particle_system_effects -> list -> start ;
						while( node )
						{
							ptr = (PARTICLE *)node -> ptr ;
							if( ptr -> lifetime != -1 )
							{
								ptr -> lifetime -= delta_time/1000.0 ;
								if( ptr -> lifetime == -1 )
									ptr -> lifetime = 0 ;
							}
							if( ptr -> lifetime > 0 || ptr -> lifetime == -1 )
							{
								animate_physics( level, &ptr -> object, level -> friction, delta_time );
								node = node -> next ;
							}
							else
							{
								LIST_NODE *node_to_kill = node ;
								node = node -> next ;
								ptr = remove_list_node( level -> particle_system_effects -> list, node_to_kill, PARTICLE );
								Free( ptr );
							}
						}
					}
				}

				if( nb_health_potions < player .attr . nb_health_potions )
				{
					N_STR *tmp_str = NULL ;
					nstrprintf( tmp_str , "Got Health potion" );
					set_gui_message( _nstr( tmp_str ) );
					Free( tmp_str );
					save_player_state( &player , "player_state.json" );
					for( int it = 0 ; it < 40; it ++ )
						{
							PHYSICS tmp_part ;
							tmp_part . sz = rand()%3  ;
							tmp_part . type = 2 ;
							VECTOR3D_SET( tmp_part . speed, 100 - rand()%200, 0.0 , 0.0  );
							VECTOR3D_SET( tmp_part . position, player . physics . position[ 0 ] + 5 - rand()%10 , player . physics . position[ 1 ] -rand()%10 , 0.0  );
							VECTOR3D_SET( tmp_part . acceleration, 0.0, 0.0, 0.0 );
							VECTOR3D_SET( tmp_part . orientation, 0.0, 0.0, 0.0 );
							add_particle( level -> particle_system_effects, -1, PIXEL_PART, 500 + rand()%500, 1+rand()%5, al_map_rgba( 100 + rand()%155 , 0 , 0 , rand()%255 ), tmp_part );
						}

				}
				if( nb_health_potions >  player .attr . nb_health_potions )
				{
					N_STR *tmp_str = NULL ;
					nstrprintf( tmp_str , "Used Health potion" );
					set_gui_message( _nstr( tmp_str ) );
					Free( tmp_str );
					save_player_state( &player , "player_state.json" );
						for( int it = 0 ; it < 40; it ++ )
						{
							PHYSICS tmp_part ;
							tmp_part . sz = rand()%3  ;
							tmp_part . type = 2 ;
							VECTOR3D_SET( tmp_part . speed, 0.0 , -10 -rand()%200, 0.0  );
							VECTOR3D_SET( tmp_part . position, player . physics . position[ 0 ] + 5 - rand()%10 , player . physics . position[ 1 ] -rand()%10 , 0.0  );
							VECTOR3D_SET( tmp_part . acceleration, 0.0, 0.0, 0.0 );
							VECTOR3D_SET( tmp_part . orientation, 0.0, 0.0, 0.0 );
							int color = 100 + rand()%155 ;
							add_particle( level -> particle_system_effects, -1, PIXEL_PART, rand()%500, rand()%3, al_map_rgba( color , 255 - color , color , rand()%255 ), tmp_part );
						}

				}
				nb_health_potions = player .attr . nb_health_potions ;
				if( nb_shield_potions < player .attr . nb_shield_potions  )
				{
					N_STR *tmp_str = NULL ;
					nstrprintf( tmp_str , "Got Shield potion" );
					set_gui_message( _nstr( tmp_str ) );
					Free( tmp_str );
					save_player_state( &player , "player_state.json" );
					for( int it = 0 ; it < 40; it ++ )
						{
							PHYSICS tmp_part ;
							tmp_part . sz = rand()%3  ;
							tmp_part . type = 2 ;
							VECTOR3D_SET( tmp_part . speed, 100 - rand()%200, 0.0 , 0.0  );
							VECTOR3D_SET( tmp_part . position, player . physics . position[ 0 ] + 5 - rand()%10 , player . physics . position[ 1 ] -rand()%10 , 0.0  );
							VECTOR3D_SET( tmp_part . acceleration, 0.0, 0.0, 0.0 );
							VECTOR3D_SET( tmp_part . orientation, 0.0, 0.0, 0.0 );
							add_particle( level -> particle_system_effects, -1, PIXEL_PART, 500 + rand()%500, 1+rand()%5, al_map_rgba( 0 , 0 , 100 + rand()%155 , rand()%255 ), tmp_part );
						}

				}
				if( nb_shield_potions > player .attr . nb_shield_potions  )
				{
					N_STR *tmp_str = NULL ;
					nstrprintf( tmp_str , "Used Shield potion" );
					set_gui_message( _nstr( tmp_str ) );
					Free( tmp_str );
					save_player_state( &player , "player_state.json" );
					for( int it = 0 ; it < 40; it ++ )
						{
							PHYSICS tmp_part ;
							tmp_part . sz = rand()%3  ;
							tmp_part . type = 2 ;
							VECTOR3D_SET( tmp_part . speed, 0.0 , -10 -rand()%200, 0.0  );
							VECTOR3D_SET( tmp_part . position, player . physics . position[ 0 ] + 5 - rand()%10 , player . physics . position[ 1 ] -rand()%10 , 0.0  );
							VECTOR3D_SET( tmp_part . acceleration, 0.0, 0.0, 0.0 );
							VECTOR3D_SET( tmp_part . orientation, 0.0, 0.0, 0.0 );
							int color = 100 + rand()%155 ;
							add_particle( level -> particle_system_effects, -1, PIXEL_PART, rand()%500, rand()%3, al_map_rgba( 0 , color , color , rand()%255 ), tmp_part );
						}

				}
				nb_shield_potions = player .attr . nb_shield_potions ;
				if( player_xp != player . attr . xp )
				{
					save_player_state( &player , "player_state.json" );
					player_xp = player . attr . xp ;
				}
				PHYSICS tmp_part ;

				if( player . attr . action == 2 )
				{
					player . attr . action = 3 ;

					switch( player . attr . current_weapon )
					{
						case 0:
							{
								if( weapon_current_angle < 0 )
								{
									weapon_start_angle = 0.0 ;
									weapon_current_angle = weapon_start_angle ;
									weapon_end_angle = 360.0 ;
								}
							}
							break;
						case 1:
							{
								if( weapon_current_angle < 0 )
								{
									weapon_start_angle = 0.0 ;
									weapon_current_angle = weapon_start_angle ;
									weapon_end_angle = 360.0 ;
								}
							}
							break;
						case 2:
							{
								double itx = 0, ity = 0, itz = 0, sz = 0;
								itx = (double)(mx - level -> native_w / 2  ) - level -> tilew / 2 ;
								ity = (double)(my - level -> native_h / 2  ) - level -> tileh / 2 ;
								itz = 0 ;
								sz = sqrt((itx * itx) + (ity * ity) + (itz * itz));
								itx /= sz ;
								ity /= sz ;
								itz /= sz ;
								tmp_part . sz = 2+rand()%10 ;
								tmp_part . type = 2 ;
								double speed_mult = 500 +rand()%100 ;
								double accuity = 20 / rand()%player . attr . level ;
								VECTOR3D_SET( tmp_part . speed, accuity + player . physics . speed[ 0 ] + itx * speed_mult, accuity + player . physics . speed[ 1 ] + ity * speed_mult, player . physics . speed[ 2 ] + itz * speed_mult );
								VECTOR3D_SET( tmp_part . position, player . physics . position[ 0 ], player . physics . position[ 1 ] - player . physics . sz , 0.0  );
								VECTOR3D_SET( tmp_part . acceleration, 0.0, 0.0, 0.0 );
								VECTOR3D_SET( tmp_part . orientation, 0.0, 0.0, 0.0 );
								int color = 100 + rand()%155 ;
								add_particle( level -> particle_system_bullets, -1, PIXEL_PART, 2000, 5 + rand()%player.attr.level ,  al_map_rgba(  0 , color , color , 100 + rand()%155 ), tmp_part );
							}
							break;
						case 3:
							{
								double itx = 0, ity = 0, itz = 0, sz = 0;
								itx = (double)(mx - level -> native_w / 2  ) - level -> tilew / 2 ;
								ity = (double)(my - level -> native_h / 2  ) - level -> tileh / 2 ;
								itz = 0 ;
								sz = sqrt((itx * itx) + (ity * ity) + (itz * itz));
								itx /= sz ;
								ity /= sz ;
								itz /= sz ;
								tmp_part . sz = 2+rand()%10 ;
								tmp_part . type = 2 ;
								for( int it = 0  ; it < player. attr . level ; it ++ )
								{
									double speed_mult = 500 +rand()%100 ;
									double accuity = 20 / rand()%player . attr . level ;
									VECTOR3D_SET( tmp_part . speed, accuity + player . physics . speed[ 0 ] + itx * speed_mult, accuity + player . physics . speed[ 1 ] + ity * speed_mult, player . physics . speed[ 2 ] + itz * speed_mult );
									VECTOR3D_SET( tmp_part . position, player . physics . position[ 0 ], player . physics . position[ 1 ] - player . physics . sz , 0.0  );
									VECTOR3D_SET( tmp_part . acceleration, 0.0, 0.0, 0.0 );
									VECTOR3D_SET( tmp_part . orientation, 0.0, 0.0, 0.0 );
									add_particle( level -> particle_system_bullets, -1, PIXEL_PART, 2000, 5 + rand()%player.attr.level,  al_map_rgba(  0 , 0 , 100 + rand()%155, rand()%155 ), tmp_part );
								}
							}
							break;
						default:
							break;
					}
				}
				if( weapon_current_angle >= 0 )
				{
					double angle_it =  ( 720  * delta_time ) / 1000000.0 ;
					if( weapon_current_angle > weapon_end_angle )
					{
						weapon_current_angle -= angle_it ;
						if( weapon_current_angle <= weapon_end_angle )
							weapon_current_angle = -1 ;
					}
					else if( weapon_current_angle < weapon_end_angle )
					{
						weapon_current_angle += angle_it ;
						if( weapon_current_angle >= weapon_end_angle )
							weapon_current_angle = -1 ;
					}
					double rad_angle = ( weapon_current_angle * ALLEGRO_PI ) / 180.0 ;
					double vx = cos( rad_angle );
					double vy = sin( rad_angle );
					for( int i = 0 ; i < player . attr . level ; i ++ )
					{
						double randpart = ( 20 + rand()%40 ) ;
						int px = player . physics . position[ 0 ]  + vx * randpart ;
						int py = player . physics . position[ 1 ] - player . anim . lib -> gfxs[ player . anim . id ] -> h  / 2  + vy * randpart ;

						int cellx = px / level -> tilew ;
						int celly = py / level -> tileh ;
						if( level -> cells[ cellx ][ celly ] >= PASSING_CELL_1 && level -> cells[ cellx ][ celly ] <= PASSING_CELL_2 )
						{
							PHYSICS tmp_part ;
							tmp_part . type = 0 ;
							VECTOR3D_SET( tmp_part . speed, 1 - rand()%2, 1 - rand()%2, 0.0  );

							VECTOR3D_SET( tmp_part . position, px , py , 0.0  );
							VECTOR3D_SET( tmp_part . acceleration, 0.0, 0.0, 0.0 );
							VECTOR3D_SET( tmp_part . orientation, 0.0, 0.0, 0.0 );
							if( player . attr . current_weapon == 0 )
							{
								int color = rand()%128 ;
								tmp_part . sz = 1 + rand() %2  ;
								if( rand()%50 > 2 )
									add_particle( level -> particle_system_bullets, -1, PIXEL_PART, 150 + rand()%100, tmp_part . sz , al_map_rgba( color , color , 0, 0 ), tmp_part );
							}
							else
							{
								int color = rand()%128 ;
								tmp_part . sz = 2 + rand() %4  ;
								add_particle( level -> particle_system_bullets, -1, PIXEL_PART, 150 + rand()%100, tmp_part . sz , al_map_rgba( 0 , 0 , color , 0 ), tmp_part );
							}
						}
					}
				}
				list_foreach( node, level -> monster_list )
				{
					MONSTER *npc = (MONSTER *)node -> ptr ;
					int px = player . physics . position[ 0 ] ;
					int py = player . physics . position[ 1 ] ;
					int npx = npc -> physics . position[0];
					int npy = npc -> physics . position[1];
					update_anim( &npc -> anim , delta_time );
					if( npx >= px - player . physics . sz && npx <= px + player . physics . sz && npy >= py - player . physics . sz && npy <= py + player . physics . sz && npc -> attr . action != 2 && player . attr . have_finished == 0 )
					{
						if( player . attr . shield > 0 )
						{
							player . attr . shield -= npc -> attr . type ;
							N_STR *tmp_str = NULL ;
							nstrprintf( tmp_str , "!! Lost %d shield points !!" , npc -> attr . type );
							set_gui_message( _nstr( tmp_str ) );
							Free( tmp_str );
							if( player . attr . shield < 0 ) player . attr . shield = 0 ;
						}
						else
						{
							player . attr . life -= npc -> attr . type ;
							N_STR *tmp_str = NULL ;
							nstrprintf( tmp_str , "!! Lost %d life points !!" , npc -> attr . type );
							set_gui_message( _nstr( tmp_str ) );
							Free( tmp_str );
						}

						npc -> physics . speed[0] = -npc -> physics . speed[0];
						npc -> physics . speed[1] = -npc -> physics . speed[1];
						npc -> attr . next_move_timer += 500000 ;
						npc -> attr . action = 2 ;


						if( player . attr . life <= 0 )
						{
							DONE = 1 ;
						}
						npx =  player . physics . position[ 0 ] ;
						npy =  player . physics . position[ 1 ] ;
						for( int it = 0 ; it < 20; it ++ )
						{
							PHYSICS tmp_part ;
							tmp_part . sz = 5  ;
							tmp_part . type = 2 ;
							VECTOR3D_SET( tmp_part . speed, 100 - rand()%200, 100 - rand()%200, 0.0  );
							VECTOR3D_SET( tmp_part . position, px, py, 0.0  );
							VECTOR3D_SET( tmp_part . acceleration, 0.0, 0.0, 0.0 );
							VECTOR3D_SET( tmp_part . orientation, 0.0, 0.0, 0.0 );
							add_particle( level -> particle_system_effects, -1, PIXEL_PART, 500 + rand()%500, 1+rand()%5, al_map_rgba( rand()%255, 0 , 0, 100 + rand()%155 ), tmp_part );
						}
					}
				}

				int cellx = player . physics . position[ 0 ] / level -> tilew ;
				int celly = player . physics . position[ 1 ] / level -> tileh ;
				if( cellx >= 0 && cellx < level -> w && celly >= 0 && celly < level -> h )
				{
					/* cell containing an exit */
					switch( level -> cells[ cellx ][ celly ] )
					{
						case EXIT_MAIN_CELL :
							nstrprintf( player . attr . level_string , "DATA/Levels/main.txt" );
							DONE = 2 ;
							n_log( LOG_INFO , "Used a door, Loading level %s" , _nstr( player . attr . level_string ) );
							break ;
						case EXIT_D1_CELL :
							nstrprintf( player . attr . level_string , "DATA/Levels/dungeon_1.txt" );
							DONE = 2 ;
							n_log( LOG_INFO , "Used a door, Loading level %s" , _nstr( player . attr . level_string ) );
							break ;
						case EXIT_D2_CELL :
							nstrprintf( player . attr . level_string , "DATA/Levels/dungeon_2.txt" );
							DONE = 2 ;
							n_log( LOG_INFO , "Used a door, Loading level %s" , _nstr( player . attr . level_string ) );
							break ;
						case EXIT_D3_CELL :
							nstrprintf( player . attr . level_string , "DATA/Levels/dungeon_3.txt" );
							DONE = 2 ;
							n_log( LOG_INFO , "Used a door, Loading level %s" , _nstr( player . attr . level_string ) );
							break ;
						case EXIT_D4_CELL :
							{
								int nb_fragments = 0 ;
								static int msg_time_counter = 0 ;
								if( player . attr . have_key_piece_1 ) nb_fragments ++ ;
								if( player . attr . have_key_piece_2 ) nb_fragments ++ ;
								if( player . attr . have_key_piece_3 ) nb_fragments ++ ;

								if( nb_fragments < 3 )
								{
									if( msg_time_counter <= 0 )
									{
										msg_time_counter = 1000000;
										N_STR *tmp_str = NULL ;
										nstrprintf( tmp_str , "Cannot enter: got only %d/3 key parts" , nb_fragments );
										set_gui_message( _nstr( tmp_str ) );
										Free( tmp_str );
									}
									else
										msg_time_counter -= delta_time ;
								}
								else
								{
									nstrprintf( player . attr . level_string , "DATA/Levels/dungeon_4.txt" );
									set_gui_message( strdup( "ENTERING LAST DUNGEON..." ) );
									DONE = 2 ;
									n_log( LOG_INFO , "Used a door, Loading level %s" , _nstr( player . attr . level_string ) );
								}
							}
							break ;
						case SWORD_CELL :
							if( player . attr . have_sword == 0 )
							{
								player . attr . have_sword = 1 ;
								set_gui_message( strdup( "Got Sword !" ) );
							}
							break ;
						case BOW_CELL :
							if( player . attr . have_magic_bow == 0 )
							{
								player . attr . have_magic_bow = 1 ;
								set_gui_message( strdup( "Got Infinite Arrow Magic Bow !" ) );
							}
							break ;
						case WAND_CELL :
							if( player . attr . have_magic_wand == 0 )
							{
								player . attr . have_magic_wand = 1 ;
								set_gui_message( strdup( "Got Magic Wand !" ) );
							}
							break ;
						case SHIELD_CELL :
							if( player . attr . have_shield == 0 )
							{
								set_gui_message( strdup( "Got Shield !" ) );
								player . attr . shield = 500 ;
								player . attr . have_shield = 1 ;
							}
							else
							{
								if( player . attr . shield < 500 )
								{
									player . attr . shield = 500 ;
									set_gui_message( strdup( "Shield got repaired" ) );
								}
							}

							break ;
						case KEY_FRAG1 :
							if( player . attr . have_key_piece_1 == 0 )
							{
								player . attr . have_key_piece_1 = 1 ;
								set_gui_message( strdup( "Got Key Fragment 1 !" ) );
								save_player_state( &player , "player_state.json" );
							}
							break ;
						case KEY_FRAG2 :
							if( player . attr . have_key_piece_2 == 0 )
							{
								player . attr . have_key_piece_2 = 1 ;
								set_gui_message( strdup( "Got Key Fragment 2 !" ) );
								save_player_state( &player , "player_state.json" );
							}
							break ;
						case KEY_FRAG3 :
							if( player . attr . have_key_piece_3 == 0 )
							{
								player . attr . have_key_piece_3 = 1 ;
								set_gui_message( strdup( "Got Key Fragment 3 !" ) );
								save_player_state( &player , "player_state.json" );
							}
							break ;
					}
				}
				if( player . attr . have_finished > 0 )
				{
					player . attr . have_finished -- ;
					if( player . attr . have_finished == 1 )
						DONE = 2 ;
				}

				do_logic = 0 ;
			}

			if( do_draw == 1 )
			{
				al_set_target_bitmap( scrbuf );
				al_clear_to_color( al_map_rgba( 0, 0, 0, 255 ) );

				draw_level( level, &player , WIDTH, HEIGHT );


				// draw weapon if in use
				if( weapon_current_angle > 0 )
				{
					if( player . attr . current_weapon == 0 )
					{
						float w = al_get_bitmap_width( wood_stick_bmp );
						float h = al_get_bitmap_height( wood_stick_bmp);
						al_draw_rotated_bitmap( wood_stick_bmp , w / 2, h , WIDTH/2, HEIGHT/2 -  player . anim . lib -> gfxs[ player . anim . id ] -> h  / 2  , (weapon_current_angle * ALLEGRO_PI) / 180.0 , 0 );
					}
					if( player . attr . current_weapon == 1 )
					{
						float w = al_get_bitmap_width( sword_bmp );
						float h = al_get_bitmap_height( sword_bmp);
						al_draw_rotated_bitmap( sword_bmp      , w / 2, h , WIDTH/2, HEIGHT/2 - player . anim . lib -> gfxs[ player . anim . id ] -> h / 2 , (weapon_current_angle * ALLEGRO_PI) / 180.0 , 0 );
					}
				}

				/* mouse pointer */
				int real_mx = mx - player . physics . sz ;
				int real_my = my - player . physics . sz ;
				al_draw_line( real_mx - 12 , real_my     , real_mx + 12 , real_my     , al_map_rgb( 255, 255, 255 ), 2 );
				al_draw_line( real_mx     , real_my + 12 , real_mx     , real_my - 12 , al_map_rgb( 255, 255, 255 ), 2 );
				al_draw_line( real_mx - 6 , real_my - 6 , real_mx + 6 , real_my + 6 , al_map_rgb( 255, 255, 255 ), 1 );
				al_draw_line( real_mx - 6 , real_my + 6 , real_mx + 6 , real_my - 6 , al_map_rgb( 255, 255, 255 ), 1 );

				/* speed meter */
				if( get_log_level() == LOG_DEBUG )
				{
					al_draw_filled_rectangle( 20, HEIGHT/2, 25, HEIGHT/2 + player . physics . speed[ 1 ], al_map_rgba( 255, 255, 255, 255 ) );
					al_draw_filled_rectangle( WIDTH/2, HEIGHT -20, WIDTH/2 + player . physics . speed[ 0 ], HEIGHT-25, al_map_rgba( 255, 255, 255, 255 ) );
				}

				al_draw_filled_rectangle( 15, 0 , WIDTH - 15 , 25 , al_map_rgba( 0 , 0 , 0 , 200 ) );
				al_draw_filled_rectangle( 17, 0 , WIDTH - 17 , 23 , al_map_rgba( 50 , 50 , 50 , 150 ) );

				N_STR *nstr = new_nstr( 512 );
				nstrprintf_cat( nstr, "Life: %d lvl:%d %d/%dxp" , player . attr . life , player . attr . level , player . attr . xp , player . attr . xp_to_level );
				int life = player . attr . life + player . attr . shield ;
				if( life > 255 ) life = 255 ; 
				al_draw_text( font, al_map_rgb( 255 - ( 255 * life ) / 255 , ( 255 *  life ) / 255 , 0 ) , 20 , 5 , 0, _nstr( nstr ) );
				empty_nstr( nstr );

				if( player . attr . have_shield == 1 )
				{
					nstrprintf_cat( nstr, " Shield: %d " , player . attr . shield );
					ALLEGRO_BITMAP *bmp = level -> tiles[ SHIELD_CELL ];
					if( bmp )
					{
						int w = al_get_bitmap_width( bmp );
						int h = al_get_bitmap_height( bmp );

						al_draw_bitmap( bmp, WIDTH - 4 * w , HEIGHT - 2 * h , 0 );
						al_draw_rectangle( WIDTH - 4 * w , HEIGHT - 2 * h , WIDTH - 3 * w , HEIGHT - h, al_map_rgb( 255, 255, 255 ) , 1.0 );
					}
				}
				if( player . attr . nb_health_potions > 0 )
				{
					nstrprintf_cat( nstr, "   Health Potions: %d " , player . attr . nb_health_potions );
				}
				if( player . attr . nb_shield_potions > 0 )
				{
					nstrprintf_cat( nstr, "   Shield Potions: %d " , player . attr . nb_shield_potions );
				}
				if( player . attr . have_key_piece_1 > 0 )
				{
					nstrcat_bytes( nstr, "Key1 " );
				}
				if( player . attr . have_key_piece_2 > 0 )
				{
					nstrcat_bytes( nstr, "Key2 " );
				}
				if( player . attr . have_key_piece_3 > 0 )
				{
					nstrcat_bytes( nstr, "Key3 " );
				}


				/* drawing icons */
				switch( player . attr . current_weapon )
				{
					case 0:
						{
							nstrcat_bytes( nstr, " Weapon: wood stick" );
							ALLEGRO_BITMAP *bmp = level -> tiles[ player . attr . current_weapon + 8 ];
							if( bmp )
							{	int w = al_get_bitmap_width( bmp );
								int h = al_get_bitmap_height( bmp );
								al_draw_rectangle( WIDTH - 2 * w , HEIGHT - 2 * h , WIDTH - w , HEIGHT - h, al_map_rgb( 255, 255, 255 ) , 1.0 );
							}
						}
						break;
					case 1:
						{
							nstrcat_bytes( nstr, " Weapon: sword" );
							ALLEGRO_BITMAP *bmp = level -> tiles[ player . attr . current_weapon + 8 ];
							if( bmp )
							{
								int w = al_get_bitmap_width( bmp );
								int h = al_get_bitmap_height( bmp );

								al_draw_bitmap( bmp, WIDTH - 2 * w , HEIGHT - 2 * h , 0 );
								al_draw_rectangle( WIDTH - 2 * w , HEIGHT - 2 * h , WIDTH - w , HEIGHT - h, al_map_rgb( 255, 255, 255 ) , 1.0 );
							}

						}
						break;
					case 2:
						{
							nstrcat_bytes( nstr, " Weapon: bow" );
							ALLEGRO_BITMAP *bmp = level -> tiles[ player . attr . current_weapon + 8 ];
							if( bmp )
							{
								int w = al_get_bitmap_width( bmp );
								int h = al_get_bitmap_height( bmp );

								al_draw_bitmap( bmp, WIDTH - 2 * w , HEIGHT - 2 * h , 0 );
								al_draw_rectangle( WIDTH - 2 * w , HEIGHT - 2 * h , WIDTH - w , HEIGHT - h, al_map_rgb( 255, 255, 255 ) , 1.0 );
							}

						}
						break;
					case 3:
						{
							nstrcat_bytes( nstr, " Weapon: magic" );
							ALLEGRO_BITMAP *bmp = level -> tiles[ player . attr . current_weapon + 8 ];
							if( bmp )
							{
								int w = al_get_bitmap_width( bmp );
								int h = al_get_bitmap_height( bmp );

								al_draw_bitmap( bmp, WIDTH - 2 * w , HEIGHT - 2 * h , 0 );
								al_draw_rectangle( WIDTH - 2 * w , HEIGHT - 2 * h , WIDTH - w , HEIGHT - h, al_map_rgb( 255, 255, 255 ) , 1.0 );
							}

						}
						break;
				}

				al_draw_filled_rectangle( 15, HEIGHT , WIDTH - 15 , HEIGHT - 25 , al_map_rgba( 0 , 0 , 0 , 200 ) );
				al_draw_filled_rectangle( 17, HEIGHT , WIDTH - 17 , HEIGHT - 23 , al_map_rgba( 50 , 50 , 50 , 150 ) );
				if( life > 255 ) life = 255 ;
				al_draw_text( font, al_map_rgb( 255 - ( 255 * life ) / 255 , ( 255 *  life ) / 255 , 0 ) , 20, HEIGHT - 20, 0, _nstr( nstr ) );
				free_nstr( &nstr );

				/* draw press up message */
				if( intro_message_delay > 0 )
				{
					intro_message_delay -= ( 1000000.0 /60.0 );
					static int color = 0 ;
					al_draw_text( font, al_map_rgb( color, color, color ), WIDTH / 2 , HEIGHT / 2 - 50       , ALLEGRO_ALIGN_CENTRE , "*** KHELDA II ***" );
					al_draw_text( font, al_map_rgb( color, color, color ), WIDTH / 2 , HEIGHT / 2 + 50 , ALLEGRO_ALIGN_CENTRE , "*** FOR AMARILLION ***" );
					color += 12 ;
					if( color > 255 )
						color = 0 ;
				}

				/* draw up tmp gui message */
				if( gui_message_delay > 0 )
				{
					gui_message_delay -= delta_time ;
					al_draw_text( font, al_map_rgb( 255, 255, 0 ), gui_message_x , gui_message_y , ALLEGRO_ALIGN_CENTRE , _str( gui_message ) );
					gui_message_y -= 2 ;
				}

				al_acknowledge_resize( display );
				int w = al_get_display_width(  display );
				int h = al_get_display_height( display );

				al_set_target_bitmap( al_get_backbuffer( display ) );

				al_clear_to_color( al_map_rgba( 0, 0, 0, 255 ) );
				al_draw_bitmap( scrbuf, w/2 - al_get_bitmap_width( scrbuf ) /2, h/2 - al_get_bitmap_height( scrbuf ) / 2, 0 );



				al_flip_display();
				do_draw = 0 ;
			}

		}
		while( !key[KEY_ESC] && !DONE );

		if( player . attr . have_finished == 1 )
		{
			DONE = 2 ;
			N_STR *str = NULL;
			nstrprintf( str, "CONGRATULATION ! You saved princess Khelda, you're now the owner of a precious Khelda Kiss !!\nYou're going to be teleported to the main map, you can enjoy and replay the world like you want" );
			al_show_native_message_box(  al_get_current_display(),
					"KheldaII",
					"Game Finished", _nstr( str ), NULL, 0  );
			/* Set the locale to the POSIX C environment */
			setlocale (LC_ALL, "POSIX");
			free_nstr( &str );
			nstrprintf( player . attr . level_string , "DATA/Levels/main.txt" );
			player . attr . have_finished = 0 ;
		}
		if( DONE == 1 )
		{
			N_STR *str = NULL;
			nstrprintf( str, "You nearly lost your life and were saved before the hit could kill you. You may have lost things." );
			al_show_native_message_box(  al_get_current_display(),
					"KheldaII",
					"Respawn", _nstr( str ), NULL, 0  );
			/* Set the locale to the POSIX C environment */
			setlocale (LC_ALL, "POSIX");
			free_nstr( &str );
			player . attr . life = 100 ;
			if( player . attr . have_shield == 1 )
				player . attr . shield = 500 ;
			player . attr . xp = 0 ;
			nstrprintf( player . attr . level_string , "DATA/Levels/main.txt" );
		}

		/******************************************************************************
		 *                             EXITING                                        *
		 ******************************************************************************/
		list_destroy( &active_object );

		save_player_state( &player , "player_state.json" );

	}while( !key[KEY_ESC] );

	return 0;
}
