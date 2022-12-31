/**\file n_iso_engine.h
 *  Map loading, saving, with objects, animations, ...
 *\author Castagnier Mickael
 *\version 1.0
 *\date 01/07/05
 */

#ifndef NILOREA_ISOMETRIC_ENGINE
#define NILOREA_ISOMETRIC_ENGINE

#ifdef __cplusplus
extern "C"
{
#endif

/**\defgroup ISOMETRIC_ENGINE GRAPHICS: isometric engine, maps, tiles, ...
   \addtogroup ISOMETRIC_ENGINE
  @{
*/

#include "n_common.h"
#include "n_log.h"

#ifndef NOALLEGRO

/*! FLAG of tile type*/
#define N_TILE    2
/*! FLAG of ability type*/
#define N_ABILITY 3
/*! FLAG of music type*/
#define N_MUSIC   4
/*! FLAG of object type*/
#define N_OBJECT  5
/*! FLAG of a walkable tile */
#define WALK 1
/*! FLAG of a swimmable tile */
#define SWIM 2
/*! FLAG of a stopping tile */
#define BLCK 3


/*! Cell of a MAP */
typedef struct CELL
{
    int /*! ident of the tile in the MAP->tile library */
    tilenumber,
    /*! ability of the tile (walking, swimming, blocking, killing ?) */
    ability,
    /*! ident of the object in the MAP->object library */
    objectnumber,
    /*! ident of the music on the tile */
    music;

} CELL;

/*! MAP with objects, tiles, skins */
typedef struct MAP
{
    /*! Grid of each cell of the map */
    CELL *grid;

    /*! Name of the map ( used for linking between two map ) */
    char *name;

    /*! Map for mouse collision between mouse pointer and map */
    BITMAP *mousemap,
           /*! defaut full colored background tile */
           *colortile;
    /*   *wiretile;       */

    int /*! size X of the grid (nbXcell) */
    XSIZE,
    /*! size Y of the grid (nbYcell) */
    YSIZE,
    /*! size X of tiles of the map (in pixel) */
    TILEW,
    /*! size Y of tiles of the map (in pixel) */
    TILEH,
    /*! X starting cell for drawing */
    ptanchorX,
    /*! Y starting cell for drawing */
    ptanchorY,
    /*! X move in pixel for drawing */
    X,
    /*! Y move in pixel for drawing */
    Y,
    /*! color of the bg */
    bgcolor,
    /*! color of wire */
    wirecolor;
    /*! active objects */
    /*OBJECT **object_lib;*/
    /*! name of the main group of the (kind of) library of objects, filled by zero else, size iz limited to 512 characaters */
    char   *object_group_names;
    /*! animation board */
    /* ANIM **anim_lib; */
    /*! as for previous:char *object_group_names */
    char *anim_group_names;
    /*! sounds library */
    /*SOUND **sounds_lib;*/
    /*! as for previous:char *object_group_names */
    char *sounds_group_names;
    /*! quest library */
    /*QUEST **quests_lib;*/
    /*! as for previous:char *object_group_names */
    char *quests_group_names;

} MAP;

/* Create an empty map */
int create_empty_map( MAP **map, char *name, int XSIZE, int YSIZE, int TILEW, int TILEH, int nbmaxobjects, int nbmaxgroup, int nbmaxanims, int nbtiles, int nbanims );
/* Set a the tilenumber of a cell */
int set_value( MAP *map, int type, int x, int y, int value );
/* Get a the tilenumber of a cell */
int get_value( MAP *map, int type, int x, int y );
/* Convert screen coordinate to map coordinate */
int ScreenToMap( int mx, int my, int *Tilex, int *Tiley, BITMAP *mousemap );
/* Center Map on given screen coordinate */
int camera_to_scr( MAP **map, int x, int y );
/* Center Map on given map coordinate, with x & y offset */
int camera_to_map( MAP **map, int tx, int ty, int x, int y );
/* Draw the map using its coordinate on the specified bitmap */
int draw_map( MAP *map, BITMAP *bmp, int destx, int desty, int mode );
/* Load the map */
int load_map( MAP **map, char *filename );
/* Save the map */
int save_map( MAP *map, char *filename );
/* Free the map */
int free_map( MAP **map );

/**
@}
*/

#endif /* NOALLEGRO */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NILOREA_ISOMETRIC_ENGINE */

