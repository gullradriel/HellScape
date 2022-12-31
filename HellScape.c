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


#include <locale.h>

#include "nilorea/n_common.h"
#include "nilorea/n_log.h"
#include "nilorea/n_list.h"
#include "nilorea/n_str.h"
#include "n_fluids.h"
#include "states_management.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_ttf.h>
#include "allegro5/allegro_audio.h"
#include "allegro5/allegro_acodec.h"
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

#define WIDTH 1920
#define HEIGHT 1080

#define RESERVED_SAMPLES   16
#define MAX_SAMPLE_DATA    10

ALLEGRO_DISPLAY *display  = NULL ;

/******************************************************************************
 *                           VARIOUS DECLARATIONS                             *
 ******************************************************************************/

int		DONE = 0,                    /* Flag to check if we are always running */
        getoptret = 0,				  /* launch parameter check */
        log_level = LOG_ERR ;			 /* default LOG_LEVEL */

ALLEGRO_TIMER *fps_timer = NULL ;
ALLEGRO_TIMER *logic_timer = NULL ;

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

    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_start_timer( fps_timer );
    al_start_timer( logic_timer );
    al_register_event_source(event_queue, al_get_timer_event_source(fps_timer));
    al_register_event_source(event_queue, al_get_timer_event_source(logic_timer));

    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_mouse_event_source());

    ALLEGRO_BITMAP *scrbuf = al_create_bitmap( WIDTH, HEIGHT );

    al_hide_mouse_cursor(display);

    enum APP_KEYS
    {
        KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_ESC, KEY_SPACE, KEY_CTRL , KEY_SHIFT , KEY_PAD_MINUS , KEY_PAD_PLUS , KEY_PAD_ENTER , KEY_M , KEY_W , KEY_F1 , KEY_F2 , KEY_F3 , KEY_F4 , KEY_F5 , KEY_F6
    };
    int key[ 19 ] = {false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false};

    if( !( sample_data[ 0 ]  = al_load_sample( "DATA/Musics/background_music.ogg" ) ) )
    {
        n_abort( "Could not load DATA/Musics/background_music.ogg" );
    }
    al_play_sample(sample_data[ 0 ] , 1 , 0 , 1 , ALLEGRO_PLAYMODE_LOOP , NULL );

    /* set fluid */
    double fluid_factor = 8.0 ;
    fluid_sim = new_n_fluid( 10000.0 , 0.0 , 40 , 0.5 , 1.9 , WIDTH / fluid_factor , HEIGHT / fluid_factor );
    
    load_fluid_state( fluid_sim , "fluid_state.json" );

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

    bool do_draw = 1 , do_logic = 1 ;
    int mx = WIDTH/3 , my = HEIGHT/2 , mouse_button = 0 , mouse_b1 = 0 , mouse_b2 = 0 ;

    al_flush_event_queue( event_queue );
    al_set_mouse_xy( display , WIDTH/3 , HEIGHT/2 );
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
                    case ALLEGRO_KEY_F5:
                        key[ KEY_F5 ] = 1 ;
                        break;
                    case ALLEGRO_KEY_F6:
                        key[ KEY_F6 ] = 1 ;
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
                    case ALLEGRO_KEY_F5:
                        key[ KEY_F5 ] = 0 ;
                        break;
                    case ALLEGRO_KEY_F6:
                        key[ KEY_F6 ] = 0 ;
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
            else
            {
                // Processing inputs 
                // get_keyboard( chat_line , ev );
                if( key[KEY_F1 ] )
                {
                    fluid_sim -> showSmoke = 1 ;
                }
                if( key[KEY_F2 ] )
                {
                    fluid_sim -> showSmoke = 0 ;
                }
                if( key[KEY_F3 ] )
                {
                    fluid_sim -> showPressure = 1 ;
                }
                if( key[KEY_F4 ] )
                {
                    fluid_sim -> showPressure = 0 ;
                }
                if( key[KEY_F5 ] )
                {
                    fluid_sim -> showPaint = 1 ;
                }
                if( key[KEY_F6 ] )
                {
                    fluid_sim -> showPaint = 0 ;
                }
                if( mouse_button )
                {
                    n_log( LOG_DEBUG , "mouse button: %d" , mouse_button );
                }
            } 
        }while( !al_is_event_queue_empty( event_queue) );

        if( do_logic == 1 )
        {
            static int old_mx = -1 , old_my = -1 ;
            double vx = 0.0 , vy = 0.0 ;
            if( old_mx != mx || old_my != my )
            {

                vx = (old_mx - mx ) / logictime ;
                vy = (old_my - my ) / logictime ;
                old_mx = mx ; 
                old_my = my ;
            }
            n_fluid_setObstacle( fluid_sim , mx / fluid_factor , ( my / fluid_factor ) - 20.0 , vx , vy , 7.0 , 1 );
            n_fluid_setObstacle( fluid_sim , (mx / fluid_factor) - 15 , my / fluid_factor , vx , vy , 10.0 , 0 );
            n_fluid_setObstacle( fluid_sim , mx / fluid_factor , ( my / fluid_factor ) + 20.0 , vx , vy , 7.0 , 0 );

            double pipeH = 0.2 * fluid_sim -> numY;
            size_t minJ = floor( 0.5 * fluid_sim -> numY - 0.5 * pipeH );
            size_t maxJ = floor( 0.5 * fluid_sim -> numY + 0.5 * pipeH );
            for (size_t j = minJ; j < maxJ; j++)
                fluid_sim -> m[j] = 0.0;

            n_fluid_simulate( fluid_sim );
            do_logic = 0 ;
        }
        if( do_draw == 1 )
        {
            al_set_target_bitmap( scrbuf );

            n_fluid_draw( fluid_sim , fluid_factor );
            al_draw_circle( mx , my - 20 * fluid_factor , 7.0 * fluid_factor , al_map_rgb( 255 , 0 , 0 ) , 2.0 );
            al_draw_circle( mx - 15 * fluid_factor , my , 10.0 * fluid_factor , al_map_rgb( 255 , 0 , 0 ) , 2.0 );
            al_draw_circle( mx , my + 20 * fluid_factor , 7.0 * fluid_factor , al_map_rgb( 255 , 0 , 0 ) , 2.0 );

            static N_STR *textout = NULL ;
            nstrprintf( textout , "[F1/F2]->showSmoke:%d [F3/F4]->showPressure:%d [F5/F6]showPaint: %d" , fluid_sim -> showSmoke , fluid_sim -> showPressure , fluid_sim -> showPaint );
            al_draw_text( font, al_map_rgb( 0 , 0 , 255 ), WIDTH / 2 , 10 , ALLEGRO_ALIGN_CENTRE , _nstr( textout ) );

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

    return 0;
}
