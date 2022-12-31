/**\file n_iso_engine.c
 *  Map loading, saving, with objects, animations, ...
 *\author Castagnier Mickael
 *\version 1.0
 *\date 04/07/05
 */

#include "nilorea/n_iso_engine.h"

#ifndef NOISOENGINE

/*!\fn create_empty_map( MAP **map , char *name , int XSIZE , int YSIZE , int TILEW , int TILEH , int nbmaxobjects , int nbmaxgroup , int nbmaxanims , int nbtiles , int nbanims )
 *\brief Create an empty map
 *\param map the map pointer to point on the allocated one
 *\param name Name of the map (used for linking between two map )
 *\param XSIZE the X size of the map (in tile)
 *\param YSIZE the Y size of the map (in tile)
 *\param TILEW the Width of a tile
 *\param TILEH the Heigh of a tile
 *\param nbmaxobjects the maximum number of objects in the map
 *\param nbmaxgroup the maximum number of active objects in the map
 *\param nbmaxanims the maximum number of aniations in the map
 *\param nbtiles the maximum number of tiles in the map
 *\param nbanims the maximum number of animations in the map
 *\return TRUE on success FALSE on failure
 */
int create_empty_map( MAP **map, char *name, int XSIZE, int YSIZE, int TILEW, int TILEH, int nbmaxobjects, int nbmaxgroup, int nbmaxanims, int nbtiles, int nbanims )
{
    int x, y;

    /* only here for warning:unused... avoiding, will be removed further */
    nbmaxobjects = nbmaxgroup = nbmaxanims = nbtiles = nbanims = 1 ;
    nbmaxgroup = nbmaxobjects ;

    /* allocating map */
    Malloc( ( *map ), MAP, 1 );

    if ( !( *map ) )
        return FALSE;

    /* allocating grid */
    Malloc( ( *map ) -> grid, CELL, XSIZE * YSIZE );

    /* allocating name */
    Malloc( ( *map ) -> name, char, ustrsizez( name ) );

    ustrcpy( ( *map ) -> name, name );

    if ( !( *map ) -> grid || !( *map ) -> name )
        return FALSE;

    ( *map ) -> XSIZE = XSIZE ;
    ( *map ) -> YSIZE = YSIZE ;

    for ( x = 0 ; x < XSIZE ; x++ )
    {
        for ( y = 0 ; y < YSIZE ; y++ )
        {
            set_value( ( *map ), N_TILE, x, y, FALSE );
            set_value( ( *map ), N_ABILITY, x, y, FALSE );
            set_value( ( *map ), N_MUSIC, x, y, FALSE );
            set_value( ( *map ), N_OBJECT, x, y, FALSE );
        }
    }

    ( *map ) -> TILEW = TILEW;
    ( *map ) -> TILEH = TILEH;
    ( *map ) -> bgcolor = makeacol( 0, 0, 0, 255 );
    ( *map ) -> wirecolor = makeacol( 255, 255, 255, 255 );

    /* Drawing the mousemap */

    ( *map ) -> mousemap = create_bitmap( ( *map ) -> TILEW, ( *map ) -> TILEH );
    ( *map ) -> colortile = create_bitmap( ( *map ) -> TILEW, ( *map ) -> TILEH );

    clear_to_color( ( *map ) -> mousemap, makecol( 255, 255, 255 ) );
    clear_to_color( ( *map ) -> colortile, ( *map ) -> bgcolor );

    /* drawing mousemap */
    line( ( *map ) -> mousemap, 0, ( TILEH >> 1 ) - 2, ( TILEW >> 1 ) - 3, 0, makecol( 255, 0, 0 ) );
    floodfill( ( *map ) -> mousemap, 1, 1, makecol( 255, 0, 0 ) );

    line( ( *map ) -> mousemap, ( TILEW >> 1 ) - 3, TILEH - 1, 0, ( TILEH >> 1 ) + 1, makecol( 0, 255, 0 ) );
    floodfill( ( *map ) -> mousemap, 1, TILEH - 2, makecol( 0, 255, 0 ) );

    line( ( *map ) -> mousemap, ( TILEW >> 1 ) + 2, TILEH - 1, TILEW - 1, ( TILEH >> 1 ) + 1, makecol( 0, 0, 255 ) );
    floodfill( ( *map ) -> mousemap, TILEW - 2, TILEH - 1, makecol( 0, 0, 255 ) );

    line( ( *map ) -> mousemap, ( TILEW >> 1 ) + 2, 0, TILEW - 1, ( TILEH >> 1 ) - 2, makecol( 255, 255, 0 ) );
    floodfill( ( *map ) -> mousemap, TILEW - 1, 0, makecol( 255, 255, 0 ) );

    /* drawing colortile */
    line( ( *map ) -> colortile, 0, ( TILEH >> 1 ) - 2, ( TILEW >> 1 ) - 3, 0, makecol( 255, 0, 255 ) );
    floodfill( ( *map ) -> colortile, 1, 1, makecol( 255, 0, 255 ) );

    line( ( *map ) -> colortile, ( TILEW >> 1 ) - 3, TILEH - 1, 0, ( TILEH >> 1 ) + 1, makecol( 255, 0, 255 ) );
    floodfill( ( *map ) -> colortile, 1, TILEH - 2, makecol( 255, 0, 255 ) );

    line( ( *map ) -> colortile, ( TILEW >> 1 ) + 2, TILEH - 1, TILEW - 1, ( TILEH >> 1 ) + 1, makecol( 255, 0, 255 ) );
    floodfill( ( *map ) -> colortile, TILEW - 2, TILEH - 1, makecol( 255, 0, 255 ) );

    line( ( *map ) -> colortile, ( TILEW >> 1 ) + 2, 0, TILEW - 1, ( TILEH >> 1 ) - 2, makecol( 255, 0, 255 ) );
    floodfill( ( *map ) -> colortile, TILEW - 1, 0, makecol( 255, 0, 255 ) );

    ( *map ) -> ptanchorX = ( *map ) -> X = 0 ;
    ( *map ) -> ptanchorY = ( *map ) -> Y = 0 ;

    return TRUE;
} /* create_empty_map( ... ) */



/*!\fn set_value( MAP *map , int type , int x , int y , int value )
 *\brief Set the value of a cell in a map
 *\param map A MAP *map to 'touch'
 *\param type It is the type of value to change , it can be N_TILE,N_ABILITY,N_MUSIC,N_OBJECT
 *\param x X coordinate of the cell
 *\param y Y coordinate of the cell
 *\param value Value to give to the cell
 *\return TRUE or FALSE
 */
int set_value( MAP *map, int type, int x, int y, int value )
{
    if ( !map || x >= map -> XSIZE || y >= map -> YSIZE )
        return FALSE;

    if ( type == N_TILE )
    {

        map -> grid[ x + y * map -> XSIZE ] . tilenumber = value ;

        return TRUE;
    }

    if ( type == N_ABILITY )
    {

        map -> grid[ x + y * map -> XSIZE ] . ability = value ;

        return TRUE;
    }

    if ( type == N_MUSIC )
    {

        map -> grid[ x + y * map -> XSIZE ] . music = value ;

        return TRUE;
    }

    if ( type == N_OBJECT )
    {

        map -> grid[ x + y * map -> XSIZE ] . objectnumber = value ;

        return TRUE;
    }


    return FALSE;
} /* set_value( ... ) */



/*!\fn get_value( MAP *map , int type , int x , int y )
 *\brief Get a the tilenumber of a cell item
 *\param map A MAP *map to 'touch'
 *\param type It is the type of value to change , it can be N_TILE,N_ABILITY,N_MUSIC,N_OBJECT
 *\param x X coordinate of the cell
 *\param y Y coordinate of the cell
 *\return The specified type value of the cell, or FALSE if unavailable
 */
int get_value( MAP *map, int type, int x, int y )
{

    if ( !map || x >= map -> XSIZE || y >= map -> YSIZE )
        return FALSE;


    if ( type == N_TILE )
        return map -> grid[ x + y * map -> XSIZE ] . tilenumber ;

    if ( type == N_OBJECT )
        return map -> grid[ x + y * map -> XSIZE ] . objectnumber ;

    if ( type == N_MUSIC )
        return map -> grid[ x + y * map -> XSIZE ] . music ;

    if ( type == N_ABILITY )
        return map -> grid[ x + y * map -> XSIZE ] . ability ;

    return FALSE;

} /* get_value( ... ) */



/*!\fn ScreenToMap( int mx, int my, int *Tilex,int *Tiley,BITMAP *mousemap)
 *\brief Convert screen coordinate to map coordinate
 *\param mx The 'x' coordinate in pixel
 *\param my The 'y' coordinate in pixel
 *\param Tilex Output for the generated Tilex
 *\param Tiley Output for the generated Tiley
 *\param mousemap A mousemap with colors for one tile
 *\return TRUE
 */
int ScreenToMap( int mx, int my, int *Tilex, int *Tiley, BITMAP *mousemap )
{

    /* SCREEN TO MAP VARIABLES */
    int RegionX, RegionY,
        RegionDX = 0, RegionDY = 0,
        MouseMapX, MouseMapY,
        MouseMapWidth, MouseMapHeight, c;

    MouseMapWidth = mousemap -> w;
    MouseMapHeight = mousemap -> h;

    /* First step find out where we are on the mousemap */
    RegionX = ( mx / MouseMapWidth );
    RegionY = ( my / MouseMapHeight ) << 1; /* The multiplying by two is very important */


    /* Second Step: Find out WHERE in the mousemap our mouse is, by finding MouseMapX and MouseMapY. */

    MouseMapX = mx % MouseMapWidth;
    MouseMapY = my % MouseMapHeight;

    /* third step: Find the color in the mouse map */
    c = getpixel( mousemap, MouseMapX, MouseMapY );

    if ( c == makecol( 255, 0, 0 ) )
    {
        RegionDX = -1;
        RegionDY = -1;
    }

    if ( c == makecol( 255, 255, 0 ) )
    {
        RegionDX = 0;
        RegionDY = -1;
    }

    if ( c == makecol( 255, 255, 255 ) )
    {
        RegionDX = 0;
        RegionDY = 0;
    }

    if ( c == makecol( 0, 255, 0 ) )
    {
        RegionDX = -1;
        RegionDY = 1;
    }

    if ( c == makecol( 0, 0, 255 ) )
    {
        RegionDX = 0;
        RegionDY = 1;
    }

    *Tilex = ( RegionDX + RegionX );
    *Tiley = ( RegionDY + RegionY );


    return TRUE;
} /* ScreenToMap( ... ) */



/*!\fn camera_to_scr( MAP **map , int x , int y )
 *\brief Center Map on given screen coordinate
 *\param map A MAP *map to center on x,y
 *\param x the X coordinate (in pixel) to center on;
 *\param y the Y coordintae (in pixel) to center on;
 *\return TRUE
 */
int camera_to_scr( MAP **map, int x, int y )
{
    if ( x < 0 )
        x = 0;

    if ( y < 0 )
        y = 0;

    ( *map ) -> X = x % ( *map ) ->TILEW;
    ( *map ) -> Y = y % ( *map ) ->TILEH;
    ( *map ) -> ptanchorY = y / ( ( *map ) ->TILEH >> 1 );
    ( *map ) -> ptanchorY = ( *map ) -> ptanchorY >> 1;
    ( *map ) -> ptanchorY = ( *map ) -> ptanchorY << 1;
    ( *map ) -> ptanchorX = ( x + ( ( *map ) -> ptanchorY & 1 ) * ( ( *map ) ->TILEW >> 1 ) ) / ( *map ) ->TILEW;

    return TRUE;
} /* camera_to_scr( ... )*/



/*!\fn camera_to_map( MAP **map , int tx , int ty , int x , int y )
 *\brief Center Map on given map coordinate, with x & y offset
 *\param map A MAP *map to center on tx+x , ty+y
 *\param tx TILEX offset in tile
 *\param ty TILEY offset in tile
 *\param x X offset in pixel
 *\param y Y Offset in pixel
 *\return TRUE
 */
int camera_to_map( MAP **map, int tx, int ty, int x, int y )
{

    ( *map ) -> ptanchorX = tx;
    ( *map ) -> ptanchorY = ty;
    ( *map ) -> X = x;
    ( *map ) -> Y = y;

    return TRUE;
} /* camera_to_map(...) */



/*!\fn draw_map( MAP *map , BITMAP *bmp , int destx , int desty , int mode)
 *\brief Draw a MAP *map on the BITMAP *bmp.
 *\param map The Map to draw
 *\param bmp The screen destination
 *\param destx X offset on bmp
 *\param desty Y offste on bmp
 *\param mode Drawing mode
 *\return TRUE
 */
int draw_map( MAP *map, BITMAP *bmp, int destx, int desty, int mode )
{
    int a, b,             /* iterators */
        x, y,             /* temp store of coordinate      */
        mvx,mvy,            /* precomputed offset */
        tw, th,           /* number of tile for one screen */
        TW2,TH2;          /* size of TILE_W /2 and TILE_H/2 */

    /* computing tw&th */
    tw = 1 + ( ( bmp -> w - destx ) / map -> TILEW ) ;
    th = 3 + ( ( bmp -> h - desty ) / ( map -> TILEH >> 1 ) ) ;

    TW2 = map -> TILEW>>1 ;
    TH2 = map -> TILEH>>1 ;

    mvx = destx - TW2 - map -> X;
    mvy = desty - TH2 - map -> Y;

    for ( a = 0 ; a <= tw ; a++ )
    {
        for ( b = 0 ; b <= th ; b++ )
        {


            x = ( a * map -> TILEW + ( ( b & 1 ) * TW2 ) ) - ( map -> TILEW >> 1 ) + mvx;
            y = ( b * TH2 ) + mvy;

            if ( ( a + map -> ptanchorX ) < map->XSIZE && ( b + map -> ptanchorY ) < map->YSIZE )
            {

                if ( map -> grid [ a + map -> ptanchorX + ( b + map -> ptanchorY ) * map->XSIZE ] . tilenumber == FALSE )
                {

                    masked_blit( map -> colortile, bmp, 0, 0, x, y, map -> colortile -> w, map -> colortile -> h );

                }

                else
                {

                    /* TODO HERE : add sprite blit */
                    triangle( bmp, x, y + ( map -> TILEH >> 1 ), x + map-> TILEW, y + ( map->TILEH >> 1 ), x + ( map -> TILEW >> 1 ), y, makecol( 255, 255, 0 ) );
                    triangle( bmp, x, y + ( map -> TILEH >> 1 ), x + map-> TILEW, y + ( map->TILEH >> 1 ), x + ( map -> TILEW >> 1 ), y + map -> TILEH, makecol( 255, 0, 255 ) );

                } /* if( map -> grid... ) ) */

                if ( mode == 1 )
                {

                    line( bmp, x + ( map -> TILEW >> 1 ) - 2, y, x, y + ( map -> TILEH >> 1 ) - 1, map -> wirecolor );
                    line( bmp, x + ( map -> TILEW >> 1 ) - 2, y + map -> TILEH - 1, x, y + ( map -> TILEH >> 1 ), map -> wirecolor );
                    line( bmp, x + ( map -> TILEW >> 1 ) + 1, y + map -> TILEH - 1, x + map -> TILEW - 1, y + ( map -> TILEH >> 1 ), map -> wirecolor );
                    line( bmp, x + ( map -> TILEW >> 1 ) + 1, y, x + map -> TILEW - 1, y + ( map -> TILEH >> 1 ) - 1, map -> wirecolor );

                }

                /* TODO HERE : check if object,
                   check if it is already in the library
                   if yes refresh its ttl
                   if no add it with the defaut ttl
                   */


            } /* if( (a + ...) ) */

        } /* for( a... ) */
    } /* for( b... ) */

    /* TODO HERE:

       sort the object list by X and by Y
       blit all the sprites of the active list
       */

    return TRUE;

} /* draw_map( ... ) */



/*!\fn load_map( MAP **map , char *filename )
 *\brief Load the map from filename
 *\param map A MAP *pointer to point on the loaded map
 *\param filename The filename of the map to load
 *\return TRUE or FALSE
 */
int load_map( MAP **map, char *filename )
{
    PACKFILE *loaded;

    char temp_name[1024];
    MAP temp_map;


    int it = 0, it1 = 0 ;

    loaded = pack_fopen( filename, "rb" );

    if( map )
        free_map( (*map) );
    if( !(*map) )
        Malloc( (*map), MAP, 1 );

    /* writting name */
    it = pack_igetl( loaded );

    pack_fread( temp_name, it, loaded );

    /* writting states */
    temp_map . XSIZE  = pack_igetl( loaded );
    temp_map . YSIZE  = pack_igetl( loaded );
    temp_map . TILEW  = pack_igetl( loaded );
    temp_map . TILEH = pack_igetl( loaded );
    temp_map . ptanchorX = pack_igetl( loaded );
    temp_map . ptanchorY = pack_igetl( loaded );
    temp_map . X = pack_igetl( loaded );
    temp_map . Y = pack_igetl( loaded );
    temp_map . bgcolor = pack_igetl( loaded );
    temp_map . wirecolor = pack_igetl( loaded );

    create_empty_map( ( *map ), temp_name,
                      temp_map . XSIZE, temp_map . YSIZE,
                      temp_map . TILEW, temp_map . TILEH,
                      2000, 2000, 2000, 2000,2000 );
    (*map ) -> XSIZE = temp_map . XSIZE ;
    (*map ) -> YSIZE = temp_map . YSIZE ;
    (*map ) -> TILEW = temp_map . TILEW ;
    (*map ) -> TILEH = temp_map . TILEH ;
    (*map ) -> ptanchorX = temp_map . ptanchorX ;
    (*map ) -> ptanchorY = temp_map . ptanchorY ;
    (*map ) -> X  = temp_map .  X ;
    (*map ) -> Y  = temp_map . Y   ;
    (*map ) ->bgcolor  = temp_map . bgcolor ;
    (*map ) ->wirecolor = temp_map . wirecolor ;


    /* saving the grid */
    for( it = 0 ; it < (*map ) -> XSIZE ; it ++ )
    {
        for( it1 = 0 ; it1 < (*map ) -> YSIZE ; it1 ++ )
        {
            (*map ) -> grid[ it + it1 * (*map ) -> XSIZE ] . tilenumber  = pack_igetl( loaded );
            (*map ) -> grid[ it + it1 * (*map ) -> XSIZE ] . ability  = pack_igetl( loaded );
            (*map ) -> grid[ it + it1 * (*map ) -> XSIZE ] . objectnumber  = pack_igetl( loaded );
            (*map ) -> grid[ it + it1 * (*map ) -> XSIZE ] . music  = pack_igetl( loaded );
        }
    }

    /*load_animlib( loaded , & (*map ) -> libtiles );*/
    /*load_animlib( loaded , & (*map ) -> libanims );*/

    pack_fclose( loaded );


    return TRUE;
} /* load_map( ... ) */



/*!\fn save_map( MAP *map , char *filename )
 *\brief Save the map to filename
 *\param map A MAP *pointer
 *\param filename The filename of the file where to save the map
 *\return TRUE or FALSE
 */
int save_map( MAP *map, char *filename )
{

    PACKFILE *saved;

    int it = 0, it1 = 0 ;

    saved = pack_fopen( filename, "wb" );

    /* writting name */
    pack_iputl( ustrsizez( map -> name ), saved );
    pack_fwrite( map -> name, ustrsizez( map -> name ), saved );

    /* writting states */
    pack_iputl( map -> XSIZE, saved );
    pack_iputl( map -> YSIZE, saved );
    pack_iputl( map -> TILEW, saved );
    pack_iputl( map -> TILEH, saved );
    pack_iputl( map -> ptanchorX, saved );
    pack_iputl( map -> ptanchorY, saved );
    pack_iputl( map -> X, saved );
    pack_iputl( map -> Y, saved );
    pack_iputl( map -> bgcolor, saved );
    pack_iputl( map -> wirecolor, saved );

    /* saving the grid */
    for( it = 0 ; it < map -> XSIZE ; it ++ )
    {
        for( it1 = 0 ; it1 < map -> YSIZE ; it1 ++ )
        {
            pack_iputl( map -> grid[ it + it1 * map -> XSIZE ] . tilenumber, saved );
            pack_iputl( map -> grid[ it + it1 * map -> XSIZE ] . ability, saved );
            pack_iputl( map -> grid[ it + it1 * map -> XSIZE ] . objectnumber, saved );
            pack_iputl( map -> grid[ it + it1 * map -> XSIZE ] . music, saved );
        }
    }

    /*save_animlib( saved , map -> libtiles );*/
    /*save_animlib( saved , map -> libanims );*/

    pack_fclose( saved );


    return TRUE;
} /* save_map( ... ) */



/*!\fn free_map( MAP **map )
 *\brief Free the map
 *\param map The map to free. The pointer will be set to NULL
 *\return TRUE or FALSE
 */
int free_map( MAP **map )
{

    if( !(*map) )
        return TRUE;
    if( (*map) -> grid )
    {
        Free( ( *map ) -> grid );
    }
    if( (*map) -> name )
    {
        Free( ( *map ) -> name );
    }
    if( (*map) -> colortile )
        destroy_bitmap( ( *map ) -> colortile );
    if( (*map) -> mousemap )
        destroy_bitmap( ( *map ) -> mousemap );

    Free( ( *map ) );

    return TRUE;
} /* free_map( ... ) */

#endif /* #ifndef NOISOENGINE */
