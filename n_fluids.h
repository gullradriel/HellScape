
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

    typedef struct N_FLUID
    {
        size_t numX ;
        size_t numY ;
        size_t numZ ;
        size_t numCells ;
        size_t numIters ;
        double density ;
        double dt ;
        double gravity ;
        double overRelaxation ;
        bool showSmoke ;
        bool showPaint ;
        bool showPressure ;
		
		double fluid_production_percentage ;
				
        double *u ;
        double *newU ;

        double *v ;
        double *newV ;

        double *p ;
        double *s ;

        double *m ;
        double *newM ;

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

    int n_fluid_set_params( N_FLUID *fluid , double gravity , double dt , size_t numIters , bool overRelaxation );

    int n_fluid_setObstacle( N_FLUID *fluid , double x , double y , double vx , double vy , double r , bool reset );

    int n_fluid_apply_obstacle_list( N_FLUID *fluid );

    ALLEGRO_COLOR n_fluid_getSciColor( double val , double minVal , double maxVal );

    int n_fluid_draw( N_FLUID *fluid , double cScale );

#ifdef __cplusplus
}
#endif

#endif
