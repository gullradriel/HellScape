/**\file n_3d.c
 *  3D helpers, vector
 *\author Castagnier Mickael
 *\version 1.0
 *\date 30/04/2014
 */

#include "nilorea/n_3d.h"
#include "math.h"


/*!\fn double distance( VECTOR3D *p1 , VECTOR3D *p2 )
 *\brief compute the distance between two VECTOR3D points
 *\param p1 The first point
 *\param p2 The second point
 *\return The computed distance
 */
double distance( VECTOR3D *p1, VECTOR3D *p2 )
{
    return sqrt( ( (*p1)[ 0 ] - (*p2)[ 0 ] ) * ( (*p1)[ 0 ] - (*p2)[ 0 ] ) +
                 ( (*p1)[ 1 ] - (*p2)[ 1 ] ) * ( (*p1)[ 1 ] - (*p2)[ 1 ] ) +
                 ( (*p1)[ 2 ] - (*p2)[ 2 ] ) * ( (*p1)[ 2 ] - (*p2)[ 2 ] ) );
} /* distance(...) */



/*!\fn int update_physics_position_nb( PHYSICS *object , int it , double delta_t )
 *\brief Update object position componant.
 *\param object The object physics to update
 *\param it Componant of VECTOR3D to update. it values : 0 -> x, 1 -> y , 2 -> z
 *\param delta_t Elapsed time since last call for componant update computing
 *\return TRUE or FALSE
 */
int update_physics_position_nb( PHYSICS *object, int it, double delta_t )
{
    __n_assert( object, return FALSE );

    object -> speed[ it ] = object -> speed[ it ] + ( object -> acceleration[ it ] * delta_t )/ 1000000.0 ;
    object -> position[ it ] = object -> position[ it ] + ( object -> speed[ it ] * delta_t ) / 1000000.0  + ( object -> acceleration[ it ] * (delta_t / 1000000.0 ) * ( delta_t / 1000000.0 ) ) / 2.0 ;
    object -> angular_speed[ it ] = object -> angular_speed[ it ] + ( object -> angular_acceleration[ it ] * delta_t ) / 1000000.0 ;
    object -> speed[ it ] = object -> speed[ it ] + ( object -> gravity[ it ] * delta_t ) / 1000000.0 ;

    return TRUE ;
} /* update_physics_position_nb(...) */



/*!\fn int update_physics_position_reverse_nb( PHYSICS *object , int it , double delta_t )
 *\brief Update object position componant, reversed.
 *\param object The object physics to update
 *\param it Componant of VECTOR3D to update. it values : 0 -> x, 1 -> y , 2 -> z
 *\param delta_t Elapsed time since last call for componant update computing
 *\return TRUE or FALSE
 */
int update_physics_position_reverse_nb( PHYSICS *object, int it, double delta_t )
{
    __n_assert( object, return FALSE );

    object -> position[ it ] = object -> position[ it ] - ( ( object -> speed[ it ] * delta_t ) / 1000000.0  + ( object -> acceleration[ it ] * (delta_t/1000000.0) * (delta_t/1000000.0 ) ) / 2.0 );
    object -> speed[ it ] = object -> speed[ it ] - ( object -> acceleration[ it ] * delta_t )/ 1000000.0 ;
    object -> angular_speed[ it ] = object -> angular_speed[ it ] - ( object -> angular_acceleration[ it ] * delta_t ) / 1000000.0 ;

    return TRUE ;
} /* update_physics_position_reverse_nb */


/*!\fn int update_physics_position_reverse( PHYSICS *object , double delta_t )
 *\brief Update object position, reversed. Wrapper of update_physics_position_reverse_nb to update each componants.
 *\param object The object physics to update
 *\param delta_t Elapsed time since last call for componant update computing
 *\return TRUE or FALSE
 */
int update_physics_position_reverse( PHYSICS *object, double delta_t )
{
    for( int it = 0 ; it < 3 ; it ++ )
    {
        object -> speed[ it ] = -object -> speed[ it ] ;
        object -> acceleration[ it ] = -object -> acceleration[ it ] ;
        object -> angular_speed[ it ] = -object -> angular_speed[ it ];

        update_physics_position_nb( object, it, delta_t );

        object -> speed[ it ] = -object -> speed[ it ] ;
        object -> acceleration[ it ] = -object -> acceleration[ it ] ;
        object -> angular_speed[ it ] = -object -> angular_speed[ it ] ;
    }
    return TRUE ;
} /* update_physics_position_reverse(...) */



/*!\fn int update_physics_position( PHYSICS *object , double delta_t )
 *\brief Update object position, reversed. Wrapper of update_physics_position_reverse_nb to update each componants.
 *\param object The object physics to update
 *\param delta_t Elapsed time since last call for componant update computing
 *\return TRUE or FALSE
 */
int update_physics_position( PHYSICS *object, double delta_t )
{
    object -> delta_t = delta_t ;
    for( int it = 0 ; it < 3 ; it ++ )
    {
        update_physics_position_nb( object, it, delta_t );
    }
    return TRUE ;
}



/*!\fn int same_sign( double a , double b )
 *\brief Quickly check if two walue are the same sign or not
 *\param a first value
 *\param b second value
 *\return TRUE or FALSE
 */
static int same_sign( double a, double b )
{
    return (( a * b) >= 0 );
} /* same_sign(...) */



/*!\fn int vector_intersect( VECTOR3D *p1 , VECTOR3D *p2 , VECTOR3D *p3 , VECTOR3D *p4 , VECTOR3D *px )
 *\brief Compute if two vectors are intersecting or not
 *\param p1 First point of vector 1
 *\param p2 Second poinf of vector 1
 *\param p3 First point of vector 2
 *\param p4 Second point of vector 2
 *\param px Storage for the eventual point
 *\return VECTOR3D_DONT_INTERSECT , VECTOR3D_COLLINEAR or VECTOR3D_DO_INTERSECT
 */
int vector_intersect(  VECTOR3D *p1, VECTOR3D *p2, VECTOR3D *p3, VECTOR3D *p4, VECTOR3D *px )
{
    double a1 = 0, a2 = 0, b1 = 0, b2 = 0, c1 = 0, c2 = 0,
           r1 = 0, r2 = 0, r3 = 0, r4 = 0,
           denom = 0, offset = 0, num = 0 ;

    /* Compute a1, b1, c1, where line joining points 1 and 2 is
       "a1 x + b1 y + c1 = 0". */
    a1 = (*p2)[ 1 ] - (*p1)[ 1 ];
    b1 = (*p1)[ 0 ] - (*p2)[ 0 ];
    c1 = ((*p2)[ 0 ] * (*p1)[ 1 ]) - ((*p1)[ 0 ] * (*p2)[ 1 ]);

    /* Compute r3 and r4. */
    r3 = ((a1 * (*p3)[ 0 ]) + (b1 * (*p3)[ 1 ]) + c1);
    r4 = ((a1 * (*p4)[ 0 ]) + (b1 * (*p4)[ 1 ]) + c1);

    /* Check signs of r3 and r4. If both point 3 and point 4 lie on
       same side of line 1, the line segments do not intersect. */
    if((r3 != 0) && (r4 != 0) && same_sign(r3, r4))
    {
        return VECTOR3D_DONT_INTERSECT;
    }

    /* Compute a2, b2, c2 */
    a2 = (*p4)[ 1 ] - (*p3)[ 1 ];
    b2 = (*p3)[ 0 ] - (*p4)[ 0 ];
    c2 = ((*p4)[ 0 ] * (*p3)[ 1 ]) - ((*p3)[ 0 ] * (*p4)[ 1 ]);

    /* Compute r1 and r2 */
    r1 = (a2 * (*p1)[ 0 ]) + (b2 * (*p1)[ 1 ]) + c2;
    r2 = (a2 * (*p2)[ 0 ]) + (b2 * (*p2)[ 1 ]) + c2;

    /* Check signs of r1 and r2. If both point 1 and point 2 lie
       on same side of second line segment, the line segments do
       not intersect. */
    if( (r1 != 0) && (r2 != 0) && same_sign(r1, r2) )
    {
        return VECTOR3D_DONT_INTERSECT;
    }

    /* Line segments intersect: compute intersection point. */
    denom = (a1 * b2) - (a2 * b1);

    if( denom == 0 )
    {
        return VECTOR3D_COLLINEAR;
    }

    if( denom < 0 )
    {
        offset = -denom / 2;
    }
    else
    {
        offset = denom / 2 ;
    }

    /* The denom/2 is to get rounding instead of truncating. It
       is added or subtracted to the numerator, depending upon the
       sign of the numerator. */
    num = (b1 * c2) - (b2 * c1);
    if(num < 0)
    {
        (*px)[ 0 ] = (num - offset) / denom;
    }
    else
    {
        (*px)[ 0 ] = (num + offset) / denom;
    }

    num = (a2 * c1) - (a1 * c2);
    if(num < 0)
    {
        (*px)[ 1 ] = ( num - offset) / denom;
    }
    else
    {
        (*px)[ 1 ] = (num + offset) / denom;
    }

    /* lines_intersect */
    return VECTOR3D_DO_INTERSECT;
} /* vector_intersect(...) */



/*!\fn double vector_dot_product( VECTOR3D *vec1, VECTOR3D *vec2 )
 *\brief Compute the dot product of two VECTOR3D
 *\param vec1 first VECTOR3D
 *\param vec2 second VECTOR3D
 *\return dot product value
 */
double vector_dot_product( VECTOR3D *vec1, VECTOR3D *vec2 )
{
    return (*vec1)[ 0 ] * (*vec2)[ 0 ] + (*vec1)[ 1 ] * (*vec2)[ 1 ] + (*vec1)[ 2 ] * (*vec2)[ 2 ];
} /* vector_dot_product(...) */



/*!\fn double vector_normalize( VECTOR3D *vec )
 *\brief Return the normalized value of vec
 *\param vec VECTOR3D to normalize
 *\return normalized value of vec
 */
double vector_normalize( VECTOR3D *vec )
{
    double res = 0.0 ;
    for( int i=0 ; i < 3 ; i++ )
    {
        res += pow( (*vec)[ i ], 2 );
    }
    return sqrt( res );
} /* vector_normalize(...) */



/*!\fn double vector_angle_between( VECTOR3D *vec1, VECTOR3D *vec2 )
 *\brief Compute angle beteen two VECTOR3D
 *\param vec1 first VECTOR3D
 *\param vec2 second VECTOR3D
 *\return angle between vectors in rad (todeg = returnval * 180/PI_2)
 */
double vector_angle_between( VECTOR3D *vec1, VECTOR3D *vec2 )
{
    return acos( vector_dot_product( &(*vec1), &(*vec2) ) / ( vector_normalize( &(*vec1) ) * vector_normalize( &(*vec2) ) ) );
} /* vector_angle_between( ... ) */


