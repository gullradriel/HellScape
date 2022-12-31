/**\file n_anim.c
* anim functions
*\author Castagnier Mickael aka Gull Ra Driel
*\version 1.0
*\date 30/12/2016
*/

#include "nilorea/n_anim.h"

#include "math.h"


/*!\fn ANIM_LIB *create_anim_library( char *name, unsigned int size )
 *\brief Allocate an animation library
 *\param name Name for the library or NULL (in which case 'generic_name' will be assigned)
 *\param size Number of animations
 *\return An allocated ANIM_LIB *object or NULL
 */
ANIM_LIB *create_anim_library( char *name, unsigned int size )
{
    if( size < 1 )
    {
        n_log( LOG_ERR, "Invalid size (<1) for anim lib creation" );
        return NULL ;
    }

    ANIM_LIB *lib = NULL ;
    Malloc( lib, ANIM_LIB, 1 );
    __n_assert( lib, return NULL );

    lib -> gfxs = NULL ;
    Malloc( lib -> gfxs, ANIM_GFX *, size );
    if( !lib -> gfxs )
    {
        n_log( LOG_ERR, "Unable to allocate a gfx_lib of size %d", size );
        return NULL ;
    }

    if( name )
        lib -> name = strdup( name );
    else
        lib -> name = strdup( "generic-name" );

    lib -> nb_max_gfxs = size ;

    return lib ;
} /* create_anim_lib */



/*!\fn int destroy_anim_lib( ANIM_LIB **lib )
 *\brief Destroy an animation library
 *\param lib animation library to destroy
 *\return TRUE or FALSE
 */
int destroy_anim_lib( ANIM_LIB **lib )
{
    __n_assert( (*lib), return FALSE );

    for( uint32_t it = 0 ; it < (*lib) -> nb_max_gfxs ; it ++ )
    {
        FreeNoLog( (*lib) -> gfxs[ it ] );
    }
    Free( (*lib) );
    return TRUE ;
} /* destroy_anim_lib */


/*!\fn int delete_bmp_from_lib( ANIM_LIB *lib, unsigned int id )
 *\brief Delete the frame at 'id' from 'lib'
 *\param lib Target animation stream
 *\param id Id of the frame to delete
 *\return TRUE or FALSE
 */
int delete_bmp_from_lib( ANIM_LIB *lib, unsigned int id )
{
    __n_assert( lib, return FALSE );
    __n_assert( lib -> gfxs, return FALSE );

    __n_assert( id, return FALSE );
    return TRUE ;
} /* delete anim from lib */



/*!\fn int add_bmp_to_lib( ANIM_LIB *lib, unsigned int pos, char *file, char *resfile )
 *\brief add a bitmap to a ANIM_LIB *lib
 *\param lib targetted library
 *\param pos position in the animation library
 *\param file input bitmap file
 *\param resfile associated input bitmap text file
 *\return TRUE or FALSE
 */
int add_bmp_to_lib( ANIM_LIB *lib, unsigned int pos, char *file, char *resfile )
{
    __n_assert( lib, return FALSE );
    __n_assert( file, return FALSE );
    __n_assert( resfile, return FALSE );

    if( pos >= lib -> nb_max_gfxs )
    {
        n_log( LOG_ERR, "invalid position %d, can only go from 0 to %d in anim lib %s", pos, lib -> nb_max_gfxs, lib -> name );
        return FALSE ;
    }
    if( lib -> gfxs[ pos ] != NULL )
    {
        n_log( LOG_ERR, "there already is a gfx at pos %d in anim lib %s", pos, lib -> name );
        return FALSE ;
    }

    FILE *data = fopen( resfile,"r");
    if( !data )
    {
        n_log( LOG_ERR, "Unable to open %s !", resfile );
    }
    int check_id ;
    fscanf( data, "%d", &check_id );
    if( check_id !=211282 )
    {
        n_log( LOG_ERR, "file %s: invalid check_id of %d, should be 211282", resfile, check_id );
        fclose( data );
        return FALSE ;
    }

    ANIM_GFX *gfx = NULL ;
    Malloc( gfx, ANIM_GFX, 1 );

    fscanf( data, "%d %d %d", &gfx -> w, &gfx -> h, &gfx -> nb_frames );

    Malloc( gfx -> frames, ANIM_FRAME, gfx -> nb_frames );

    gfx -> bmp = al_load_bitmap( file );
    if( !gfx -> bmp )
    {
        n_log( LOG_ERR, "file %s: unable to load image", file );
        fclose( data );
        return FALSE ;
    }

    for( unsigned int it = 0 ; it < gfx -> nb_frames ; it ++ )
    {
        fscanf( data, "%d %d %d", &gfx -> frames[ it ].x, &gfx -> frames[ it ].y, &gfx -> frames[ it ] . duration );
    }
    fclose( data );

    lib -> gfxs[ pos ] = gfx ;

    return TRUE ;
} /* add_bmp_to_lib( ... ) */


/*!\fn int update_anim( ANIM_DATA *data, unsigned int delta_t )
 *\brief compute and update an ANIM_DATA context based on elapsed delta_t time in usecs
 *\param data targetted ANIM_DATA
 *\param delta_t elapsed time to apply (usecs)
 *\return TRUE or FALSE
 */
int update_anim( ANIM_DATA *data, unsigned int delta_t )
{
    __n_assert( data, return FALSE );

    data -> elapsed += delta_t ;
    if( data -> elapsed > data -> lib -> gfxs[ data -> id ] -> frames[ data -> frame ].duration )
    {
        data -> elapsed = 0 ;
        data -> frame ++ ;
        if( data -> frame >= data -> lib -> gfxs[ data -> id ] -> nb_frames )
            data -> frame = 0 ;
    }
    return TRUE ;
} /* update_anim( ... ) */


/*!\fn int draw_anim( ANIM_DATA *data, int x,  int y )
 *\brief blit an ANIM_DATA at position x,y ( current selected video buffer is used )
 *\param data ANIM_DATA *source to use
 *\param x x position on screen
 *\param y y position on screen
 *\return TRUE or FALSE
 */
int draw_anim( ANIM_DATA *data, int x,  int y )
{
    ALLEGRO_BITMAP *bmp = data -> lib -> gfxs[ data -> id ] -> bmp ;


    unsigned int tilew = data -> lib -> gfxs[ data -> id ] -> w ;
    unsigned int tileh = data -> lib -> gfxs[ data -> id ] -> h ;

    int px = data -> lib -> gfxs[ data -> id ] -> frames[ data -> frame ] . x ;
    int py = data -> lib -> gfxs[ data -> id ] -> frames[ data -> frame ] . y ;

    unsigned int framex = data -> frame * tilew ;

    al_draw_bitmap_region( bmp, framex, 0, tilew, tileh, x - px, y - py, 0 );

    return TRUE ;
}
/* draw_anim( ... ) */
