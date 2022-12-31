/**\file HellScape.c
 *
 *  HellScape Main File
 *
 *\author Castagnier Mickaël
 *
 *\version 1.0
 *
 *\date 30/12/2021
 */

#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_ttf.h>
#include <locale.h>
#include "nilorea/n_common.h"
#include "nilorea/n_particles.h"
#include "nilorea/n_anim.h"
#include "level.h"
#include "states_management.h"
#include "n_fluids.h"

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

N_FLUID *fluid_sim = NULL ;

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


    al_set_new_display_flags( ALLEGRO_OPENGL|ALLEGRO_FULLSCREEN_WINDOW );
    display = al_create_display( WIDTH, HEIGHT );
    if( !display )
    {
        n_abort("Unable to create display\n");
    }
    al_set_window_title( display, "HellScape" );

    al_set_new_bitmap_flags( ALLEGRO_VIDEO_BITMAP );

    ALLEGRO_FONT *font = al_load_font( "DATA/2Dumb.ttf", 18, 0 );

    DONE = 0 ;
    double fps = 60.0 ;
    fps_timer = al_create_timer( 1.0 / fps );
    double logictime = fps * 2.0 ;
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
        }
        VECTOR3D_SET( player . physics . gravity , level -> gravity[ 0 ] , level -> gravity[ 1 ] , level -> gravity[ 2 ] );
        double shoot_rate_time = 0 ;
        VECTOR3D old_pos ;

        int mx = 0, my = 0, mouse_b1 = 0, mouse_b2 = 0 ;
        int do_draw = 0, do_logic = 0 ;
        int mouse_button = 0 ;
        DONE = 0 ;

        /* set fluid */
        double fluid_factor = 8.0 ;
        fluid_sim = new_n_fluid( 10000.0 , 0.0 , 16 , 0.5 , 1.9 , WIDTH / fluid_factor , HEIGHT / fluid_factor );

        size_t n = fluid_sim -> numY;
        double inVel = 2.0;
        for( size_t i = 0; i < fluid_sim -> numX; i++) 
        {
            for (size_t j = 0; j < fluid_sim -> numY; j++) 
            {
                double s = 1.0;	// fluid
                if (i == 0 || j == 0 || j == fluid_sim -> numY-1)
                    s = 0.0;	// solid
                                //
                fluid_sim -> s[i*n + j] = s ;

                if (i == 1) {
                    fluid_sim -> u[i*n + j] = inVel;
                }
            }
        }
        double pipeH = 0.2 * fluid_sim -> numY;
        size_t minJ = floor( 0.5 * fluid_sim -> numY - 0.5 * pipeH );
        size_t maxJ = floor( 0.5 * fluid_sim -> numY + 0.5 * pipeH );
        for (size_t j = minJ; j < maxJ; j++)
            fluid_sim -> m[j] = 0.0;

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
                /* draw fluid fuild */
                n_fluid_simulate( fluid_sim );
                static int old_mx = -1 , old_my = -1 ;
                double vx = 0.0 , vy = 0.0 ;
                if( old_mx != mx || old_my != my )
                {
                    vx = (old_mx - mx ) / logictime ;
                    vy = (old_my - my ) / logictime ;
                    old_mx = mx ; 
                    old_my = my ;
					n_fluid_setObstacle( fluid_sim , mx / fluid_factor , ( my / fluid_factor ) - 20.0 , vx , vy , 7.0 , 1 );
					n_fluid_setObstacle( fluid_sim , (mx / fluid_factor) - 15 , my / fluid_factor , vx , vy , 10.0 , 0 );
					n_fluid_setObstacle( fluid_sim , mx / fluid_factor , ( my / fluid_factor ) + 20.0 , vx , vy , 7.0 , 0 );
					size_t n = fluid_sim -> numY;
					double inVel = 2.0;
					double pipeH = 0.2 * fluid_sim -> numY;
					size_t minJ = floor( 0.5 * fluid_sim -> numY - 0.5 * pipeH );
					size_t maxJ = floor( 0.5 * fluid_sim -> numY + 0.5 * pipeH );
					for (size_t j = minJ; j < maxJ; j++)
						fluid_sim -> m[j] = 0.0;
				}
				do_logic = 0 ;
			}
            if( do_draw == 1 )
            {
                al_set_target_bitmap( scrbuf );
                n_fluid_draw( fluid_sim , fluid_factor );
                al_draw_circle( mx , my , 16.0 * fluid_factor , al_map_rgb( 255 , 0 , 0 ) , 2.0 );
                al_acknowledge_resize( display );
                int w = al_get_display_width(  display );
                int h = al_get_display_height( display );
                al_set_target_bitmap( al_get_backbuffer( display ) );
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
            nstrprintf( str, "CONGRATULATION ! You saved princess HellScape, you're now the owner of a precious HellScape Kiss !!\nYou're going to be teleported to the main map, you can enjoy and replay the world like you want" );
            al_show_native_message_box(  al_get_current_display(),
                    "HellScape",
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
                    "HellScape",
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
