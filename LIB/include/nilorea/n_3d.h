/**\file n_3d.h
 * Simple 3D movement simulation
 *\author Castagnier Mickael
 *\version 1.0
 *\date 30/04/2014
 */

#ifndef _N_3D_HEADER
#define _N_3D_HEADER

#ifdef __cplusplus
extern "C" {
#endif

/**\defgroup PHYSICS PHYSICS: simple 3D movement simulation
   \addtogroup PHYSICS
  @{
*/

#include "n_log.h"
#include "n_common.h"
#include <sys/time.h>

/*! PHYSICS object state for STOPPED */
#define MOVE_STOPPED 0
/*! PHYSICS object state for move interpolated between two position */
#define MOVE_INTER 1
/*! PHYSICS object state for move simulated from latest update */
#define MOVE_SIMU 2

/*! value when the two VECTOR3D are not connected */
#define VECTOR3D_DONT_INTERSECT -2
/*! value when the two VECTOR3D are collinear */
#define VECTOR3D_COLLINEAR -1
/*! value when the two VECTOR3D are intersecting */
#define VECTOR3D_DO_INTERSECT 0

/*! struct of a point */
typedef double VECTOR3D[ 3 ];        /* x , y , z  */


/*! helper to set a VECTOR3D position */
#define VECTOR3D_SET( VECTOR , X , Y , Z ) \
   VECTOR[ 0 ] = (X) ; \
   VECTOR[ 1 ] = (Y) ; \
   VECTOR[ 2 ] = (Z) ;

/*! structure of the physics of an object */
typedef struct PHYSICS
{
    /*! last delta_t used */
    time_t delta_t ;
    /*! x,y,z actual position */
    VECTOR3D	position ;
    /*! vx,vy,vz actual speed */
    VECTOR3D speed ;
    /*! ax,ay,az actual acceleration */
    VECTOR3D acceleration ;
    /*! ax,ay,az actual rotation position */
    VECTOR3D orientation ;
    /*! rvx,rvy,rvz actual angular speed */
    VECTOR3D angular_speed ;
    /*! rax,ray,raz actual angular acceleration */
    VECTOR3D angular_acceleration ;
    /*! gx , gy , gz gravity */
    VECTOR3D gravity ;
    /*! ability */
    int can_jump ;
    /*! size */
    int sz ;
    /*! optionnal type id */
    int type ;
} PHYSICS ;

/* distance between two points */
double distance( VECTOR3D *p1, VECTOR3D *p2 );
/* update componant[ it ] of a VECTOR3D */
int update_physics_position_nb( PHYSICS *object, int it, double delta_t );
/* update componant[ it ] of a VECTOR3D, reversed time */
int update_physics_position_reverse_nb( PHYSICS *object, int it, double delta_t );
/* update the position of an object, using the delta time T to update positions */
int update_physics_position( PHYSICS *object, double delta_t );
/* update the position of an object, using the delta time T to reverse update positions */
int update_physics_position_reverse( PHYSICS *object, double delta_t );
/* compute if two vector are colliding, storing the resulting point in px */
int vector_intersect( VECTOR3D *p0, VECTOR3D *p1, VECTOR3D *p2, VECTOR3D *p3, VECTOR3D *px );
/* dot product */
double vector_dot_product( VECTOR3D *vec1, VECTOR3D *vec2 );
/* vector normalize */
double vector_normalize( VECTOR3D *vec );
/* vector angle with other vector */
double vector_angle_between( VECTOR3D *vec1, VECTOR3D *vec2 );

/*! VECTOR3D copy wrapper */
#define copy_point( __src_ , __dst_ ) \
   memcpy( __dst_ , __src_ , sizeof( VECTOR3D ) );

/**
@}
*/

#ifdef __cplusplus
}
#endif

#endif // header guard

