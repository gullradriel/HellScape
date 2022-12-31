/**\file n_particles.c
*  particle function file for SantaHack 2012
*\author Castagnier Mickael aka Gull Ra Driel
*\version 1.0
*\date 20/12/2012
*/



#include "nilorea/n_particles.h"
#include "math.h"


/*!\fn int init_particle_system( PARTICLE_SYSTEM **psys, int max, double x, double y, double z, int max_sprites )
 *\brief initialize a particle system
 *\param psys a pointer to a NULL initialized PARTICLE_SYSTEM pointer
 *\param max maximum number of particles in the system. Set to zero or negative to disable the limitation
 *\param x x position of the emitter
 *\param y y position of the emitter
 *\param z z position of the emitter
 *\param max_sprites maximum number of sprites used in the system. Set to zero or negative if there are no sprites used.
 *\return TRUE or FALSE
 */
int init_particle_system( PARTICLE_SYSTEM **psys, int max, double x, double y, double z, int max_sprites )
{
    __n_assert( !(*psys), n_log( LOG_ERR, "particle system %p already initialized", (*psys) ); return FALSE );

    Malloc( (*psys), PARTICLE_SYSTEM, 1 );

    start_HiTimer( &(*psys) -> timer );

    (*psys) -> list = new_generic_list( max );

    (*psys) -> source[ 0 ] = x ;
    (*psys) -> source[ 1 ] = y ;
    (*psys) -> source[ 2 ] = z ;

    if( max_sprites > 0 )
        (*psys) -> sprites = (ALLEGRO_BITMAP **)calloc( max_sprites, sizeof( ALLEGRO_BITMAP *) );
    else
        (*psys) -> sprites = NULL ;

    return TRUE ;
} /* init_particle_system() */



/*!\fn int add_particle( PARTICLE_SYSTEM *psys, int spr, int mode, int lifetime, int size, ALLEGRO_COLOR color, PHYSICS object )
 *\brief add a particle to a particle system
 *\param psys targeted particle system
 *\param spr sprite id of the particle, if any. Set to negative if there is no sprite for the particle
 *\param mode particle mode, NORMAL_PART:if sprite id then use sprite, else draw a pixel,SINUS_PART: snow moving effect,PIXEL_PART: rectfill with size
 *\param lifetime duration of the particle in msecs
 *\param size size of the particle, in pixels
 *\param color color of the particle
 *\param object PHYSICS object in which you set the particle initial position, speed, acceleration, rotation
 *\return TRUE or FALSE
 */
int add_particle( PARTICLE_SYSTEM *psys, int spr, int mode, int lifetime, int size, ALLEGRO_COLOR color, PHYSICS object )
{
    int it = 0 ;

    PARTICLE *new_p = NULL ;


    if( psys -> list -> nb_items == psys -> list -> nb_max_items )
        return FALSE ;

    for( it = 0 ; it < 3 ; it ++ )
    {
        object . position[ it ] += psys -> source[ it ] ;
    }

    Malloc( new_p, PARTICLE, 1 );
    __n_assert( new_p, return FALSE );

    new_p -> spr_id   = spr ;
    new_p -> mode     = mode ;
    new_p -> lifetime = lifetime ;
    new_p -> color    = color ;
    new_p -> size    = size ;

    memcpy( new_p -> object . position, object . position, sizeof( VECTOR3D ) );
    memcpy( new_p -> object . speed, object . speed, sizeof( VECTOR3D ) );
    memcpy( new_p -> object . acceleration, object . acceleration, sizeof( VECTOR3D ) );
    memcpy( new_p -> object . angular_speed, object . angular_speed, sizeof( VECTOR3D ) );
    memcpy( new_p -> object . angular_acceleration, object . angular_acceleration, sizeof( VECTOR3D ) );
    memcpy( new_p -> object . orientation, object . orientation, sizeof( VECTOR3D ) );
    new_p -> object . sz = object . sz ;
    new_p -> object . type = object . type ;

    return list_push( psys -> list, new_p, &free );
} /* add_particle() */




/*!\fn int add_particle_ex( PARTICLE_SYSTEM *psys, int spr, int mode, int off_x, int off_y, int lifetime, int size, ALLEGRO_COLOR color, double vx, double vy, double vz, double ax, double ay, double az )
 *\brief add a particle to a particle system, all in line version (you have to set the PHYSICS object parameter in the function parameter instead of providing a PHYSICS object)
 *\param psys targeted particle system
 *\param spr sprite id of the particle, if any. Set to negative if there is no sprite for the particle
 *\param mode particle mode, NORMAL_PART:if sprite id then use sprite, else draw a pixel,SINUS_PART: snow moving effect,PIXEL_PART: rectfill with size
 *\param off_x x offset from particle source x position
 *\param off_y y offset from particle source x position
 *\param lifetime duration of the particle in msecs
 *\param size size of the particle, in pixels
 *\param color color of the particle
 *\param vx x speed of the particle
 *\param vy vy speed of the particle
 *\param vz svz peed of the particle
 *\param ax x acceleration of the particle
 *\param ay y acceleration of the particle
 *\param az z acceleration of the particle
 *\return TRUE or FALSE
 */
int add_particle_ex( PARTICLE_SYSTEM *psys, int spr, int mode, int off_x, int off_y, int lifetime, int size, ALLEGRO_COLOR color,
                     double vx, double vy, double vz,
                     double ax, double ay, double az )
{
    PHYSICS object ;
    VECTOR3D_SET( object . position, off_x, off_y, 0.0 );
    VECTOR3D_SET( object . speed,  vx, vy, vz );
    VECTOR3D_SET( object . acceleration, ax, ay, az );
    VECTOR3D_SET( object . orientation, 0.0, 0.0, 0.0 );
    VECTOR3D_SET( object . angular_speed, 0.0, 0.0, 0.0 );
    VECTOR3D_SET( object . angular_acceleration, 0.0, 0.0, 0.0 );

    return add_particle( psys, spr, mode, lifetime, size,color, object );
} /* add_particle_ex () */



/*!\fn int manage_particle_ex( PARTICLE_SYSTEM *psys , double delta_t )
 *\brief update particles positions usting provided delta time
 *\param psys the targeted particle system
 *\param delta_t delta time to use, in msecs
 *\return TRUE or FALSE
 */
int manage_particle_ex( PARTICLE_SYSTEM *psys, double delta_t )
{
    __n_assert( psys, return FALSE );

    LIST_NODE *node = NULL ;
    PARTICLE *ptr = NULL ;

    node = psys -> list -> start ;

    while( node )
    {
        ptr = (PARTICLE *)node -> ptr ;
        if( ptr -> lifetime != -1 )
        {
            ptr -> lifetime -= delta_t / 1000.0 ;
        }

        if( ptr -> lifetime > 0 || ptr -> lifetime == -1 )
        {
            update_physics_position( &ptr -> object, delta_t );
            node = node -> next ;
        }
        else
        {
            LIST_NODE *node_to_kill = node ;
            node = node -> next ;
            ptr = remove_list_node( psys -> list, node_to_kill, PARTICLE );
            Free( ptr );
        }
    }

    return TRUE;
} /* manage_particle_ex() */



/*!\fn int manage_particle( PARTICLE_SYSTEM *psys )
 *\brief update particles positions usting particle system internal timer
 *\param psys the targeted particle system
 *\return TRUE or FALSE
 */
int manage_particle( PARTICLE_SYSTEM *psys )
{
    __n_assert( psys, return FALSE );

    double delta_t = get_usec( &psys -> timer );
    return manage_particle_ex( psys, delta_t );
} /* manage_particle() */



/*!\fn int draw_particle( PARTICLE_SYSTEM *psys, double xpos, double ypos, int w, int h, double range )
 *\brief draw particles of a particle system
 *\param psys the targeted particle system
 *\param xpos camera x position
 *\param ypos camera y position
 *\param w width of the current display
 *\param h height of the current display
 *\param range display border tolerance, if( ( x < -range ) || ( x > ( w + range ) ) || ( y< -range ) || ( y > ( h + range ) ) ) next ;
 *\return TRUE or FALSE
 */
int draw_particle( PARTICLE_SYSTEM *psys, double xpos, double ypos, int w, int h, double range )
{
    __n_assert( psys, return FALSE );

    LIST_NODE *node = NULL ;
    PARTICLE *ptr = NULL ;

    node  = psys -> list -> start ;

    while( node )
    {
        double x = 0, y = 0 ;

        ptr = (PARTICLE *)node -> ptr ;
        x = ptr -> object . position[ 0 ] - xpos ;
        y = ptr -> object . position[ 1 ] - ypos ;

        if( ( x < -range ) || ( x > ( w + range ) ) || ( y< -range ) || ( y > ( h + range ) ) )
        {
            node = node -> next ;
            continue ;
        }

        for( int it = 0 ; it < 3 ; it ++ )
        {
            while( ptr -> object . orientation[ it ] < 0.0 )
                ptr -> object . orientation[ it ] += 256.0 ;

            if( ptr -> object . orientation[ it ] >= 256.0 )
                ptr -> object . orientation[ it ] = fmod(  ptr -> object . orientation[ it ], 256.0 );
        }

        if( ptr -> mode == SINUS_PART )
        {
            if( ptr -> object . speed[ 0 ] != 0 )
                x = x + ptr -> object . speed[ 0 ] * sin( (ptr -> object . position[ 0 ]/ptr -> object . speed[ 0 ]) ) ;
            else
                x = x + ptr -> object . speed[ 0 ] * sin( ptr -> object . position[ 0 ] );

            if( ptr -> object . speed[ 1 ] != 0 )
                y = y + ptr -> object . speed[ 1 ] * cos( (ptr -> object . speed[ 1 ]/ptr -> object . speed[ 1 ]) ) ;
            else
                y = y + ptr -> object . speed[ 1 ] * sin( ptr -> object . position[ 1 ] ) ;

            if( ptr -> spr_id >= 0 && ptr -> spr_id < psys -> max_sprites && psys -> sprites[ ptr -> spr_id ] )
            {
                int spr_w = al_get_bitmap_width( psys -> sprites[ ptr -> spr_id ] );
                int spr_h = al_get_bitmap_height( psys -> sprites[ ptr -> spr_id ] );

                al_draw_rotated_bitmap( psys -> sprites[ ptr -> spr_id ], spr_w/2, spr_h/2, x - spr_w / 2, y - spr_h /2, al_ftofix( ptr -> object . orientation[ 2 ]), 0 );
            }
            else
                al_draw_circle( x, y, ptr -> size, ptr -> color, 1 );
        }

        if( ptr -> mode & NORMAL_PART )
        {
            if( ptr -> spr_id >= 0 && ptr -> spr_id < psys -> max_sprites && psys -> sprites[ ptr -> spr_id ] )
            {
                int w = al_get_bitmap_width( psys -> sprites[ ptr -> spr_id ] );
                int h = al_get_bitmap_height( psys -> sprites[ ptr -> spr_id ] );

                al_draw_rotated_bitmap( psys -> sprites[ ptr -> spr_id ], w/2, h/2, x - w / 2, y - h /2, al_ftofix( ptr -> object . orientation[ 2 ]), 0 );
            }
            else
                al_draw_circle( x, y, ptr -> size, ptr -> color, 1 );
        }
        else if( ptr -> mode & PIXEL_PART )
        {
            al_draw_filled_rectangle( x - ptr -> size, y - ptr -> size, x + ptr -> size, y + ptr -> size, ptr -> color );
        }
        else
            al_draw_circle( x, y, ptr -> size, ptr -> color, 1 );
        node  = node -> next ;
    }

    return TRUE;
} /* draw_particle() */



/*!\fn int free_particle_system( PARTICLE_SYSTEM **psys)
 *\brief destroy and free a particle system
 *\param psys a pointer to the particle system to destroy
 *\return TRUE or FALSE
 */
int free_particle_system( PARTICLE_SYSTEM **psys)
{
    __n_assert( (*psys), return FALSE );

    PARTICLE *particle = NULL ;
    while( (*psys) -> list -> start )
    {
        particle = remove_list_node( (*psys) -> list, (*psys) -> list -> start, PARTICLE );
        Free( particle );
    }
    Free( (*psys) );

    return TRUE;
} /* free_particle_system() */



/*!\fn int move_particles( PARTICLE_SYSTEM *psys, double vx, double vy, double vz )
 *\brief draw particles of a particle system
 *\param psys the targeted particle system
 *\param vx x move
 *\param vy y move
 *\param vz z move
 *\return TRUE or FALSE
 */
int move_particles( PARTICLE_SYSTEM *psys, double vx, double vy, double vz )
{
    __n_assert( psys, return FALSE );

    LIST_NODE *node = NULL ;
    PARTICLE *ptr = NULL ;

    node  = psys -> list -> start ;

    while( node )
    {
        ptr = (PARTICLE *)node -> ptr ;
        ptr -> object . position[ 0 ] = ptr -> object . position[ 0 ] + vx ;
        ptr -> object . position[ 1 ] = ptr -> object . position[ 1 ] + vy ;
        ptr -> object . position[ 2 ] = ptr -> object . position[ 2 ] + vz ;
        node = node -> next ;
    }
    return TRUE ;
}
