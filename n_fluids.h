
#include "allegro5/allegro.h"
#include <allegro5/allegro_primitives.h>
/**\file n_fluids.h
 *
 *  fluid management
 *
 *\author Castagnier Mickaël aka Gull Ra Driel
 *
 *\version 1.0
 *
 *\date 31/12/2022 
 *
 */

#ifndef __N_FLUID_HEADER
#define __N_FLUID_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include "nilorea/n_list.h"
#include "nilorea/n_thread_pool.h"

#ifndef _z
#define _z( __fluid , __componant ) ( (__fluid->__componant) > (__fluid->negative_float_tolerance) && (__fluid->__componant) < (__fluid->positive_float_tolerance) )
#endif

#ifndef _zd
#define _zd( __fluid , __value ) ( (__value) > (__fluid->negative_float_tolerance) && (__value) < (__fluid->positive_float_tolerance) )
#endif


    typedef struct N_FLUID_THREAD_PARAMS
    {
        /*! pointer to data which will be used in the proc */
        void *ptr ;
        /*! x start point */
        size_t x_start ;
        /*! x end point */
        size_t x_end ;
        /*! y start point */
        size_t y_start ;
        /*! y end point */
        size_t y_end ;
    } N_FLUID_THREAD_PARAMS ;

    typedef struct N_FLUID
    {
        size_t numX ;
        size_t numY ;
        size_t numZ ;
        size_t numCells ;
        size_t numIters ;
        double density ;
        double dt ;
        double h ;
        double gravity ;
        double overRelaxation ;
        bool showSmoke ;
        bool showPaint ;
        bool showPressure ;

        double negative_float_tolerance ;
        double positive_float_tolerance ;

        double fluid_production_percentage ;
        double cScale ;

        double *u ;
        double *newU ;

        double *v ;
        double *newV ;

        double *p ;
        double *s ;

        double *m ;
        double *newM ;

        /*! preprocessed list of threaded procs parameters, for n_fluid_integrate  */
        LIST *integrate_chunk_list ;
        /*! preprocessed list of threaded procs parameters, for n_fluid_solveIncompressibility  */
        LIST *solveImcompressibility_chunk_list ;
        /*! preprocessed list of threaded procs parameters, for n_fluid_advectVel */
        LIST *advectVel_chunk_list ;
        /*! preprocessed list of threaded procs parameters, for n_fluid_advectSmoke  */
        LIST *advectSmoke_chunk_list ;

    } N_FLUID ;


    int destroy_n_fluid( N_FLUID **fluid );
    N_FLUID *new_n_fluid( double density , double gravity , size_t numIters , double dt , double overRelaxation , size_t sx , size_t sy );

    int n_fluid_integrate( N_FLUID *fluid );
    int n_fluid_solveIncompressibility( N_FLUID *fluid );
    int n_fluid_extrapolate( N_FLUID *fluid );

    double n_fluid_sampleField( N_FLUID *fluid , double x , double y , uint32_t field );
    double n_fluid_avgU( N_FLUID *fluid , size_t i , size_t j );
    double n_fluid_avgV( N_FLUID *fluid , size_t i , size_t j );

    int n_fluid_advectVel( N_FLUID *fluid );
    int n_fluid_advectSmoke( N_FLUID *fluid );

    int n_fluid_simulate( N_FLUID *fluid );
    int n_fluid_simulate_threaded( N_FLUID *fluid , THREAD_POOL *thread_pool );

    int n_fluid_setObstacle( N_FLUID *fluid , double x , double y , double vx , double vy , double r , bool reset );

    ALLEGRO_COLOR n_fluid_getSciColor( N_FLUID *fluid , double val , double minVal , double maxVal );
    int n_fluid_draw( N_FLUID *fluid );

#ifdef __cplusplus
}
#endif

#endif
