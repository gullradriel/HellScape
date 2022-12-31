#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <strings.h>


#include "n_fluids.h"
#include "nilorea/n_common.h"

#define N_FLUID_U_FIELD 0
#define N_FLUID_V_FIELD 1
#define N_FLUID_S_FIELD 2

void n_memset( void *dst , void *val , size_t size , size_t count )
{
    void *ptr = dst;
    while( count-- > 0 )
    {
        memcpy( ptr , val , size );
        ptr += size;
    }
}

int destroy_n_fluid( N_FLUID **fluid )
{
    __n_assert( (*fluid) , return FALSE );

    FreeNoLog( (*fluid) -> u );
    FreeNoLog( (*fluid) -> newU );
    FreeNoLog( (*fluid) -> v );
    FreeNoLog( (*fluid) -> newV );
    FreeNoLog( (*fluid) -> p );
    FreeNoLog( (*fluid) -> s );
    FreeNoLog( (*fluid) -> m );
    FreeNoLog( (*fluid) -> newM );
    FreeNoLog( (*fluid) );

    return TRUE ;
}

N_FLUID *new_n_fluid( double density , double gravity , size_t numIters , double dt , double overRelaxation , size_t sx , size_t sy )
{
    N_FLUID *fluid = NULL ;

    Malloc( fluid , N_FLUID , 1 );
    __n_assert( fluid , return NULL );

    fluid -> density = density ;
    fluid -> gravity = gravity ;
    fluid -> numIters = numIters ;
    fluid -> dt = dt ;
    fluid -> overRelaxation = overRelaxation ;
    fluid -> numX = sx + 2 ;
    fluid -> numY = sy + 2 ;
    fluid -> numCells = fluid -> numX * fluid -> numY ;

    Malloc( fluid -> u    , double , fluid -> numCells );
    Malloc( fluid -> newU , double , fluid -> numCells );
    Malloc( fluid -> v    , double , fluid -> numCells );
    Malloc( fluid -> newV , double , fluid -> numCells );
    Malloc( fluid -> p    , double , fluid -> numCells );
    Malloc( fluid -> s    , double , fluid -> numCells );
    Malloc( fluid -> m    , double , fluid -> numCells );
    Malloc( fluid -> newM , double , fluid -> numCells );

    double d_val = 1.0 ;
    n_memset( fluid -> m , &d_val , sizeof( d_val ) , fluid -> numCells );

    return fluid ;
} /* new_n_fluid */


int n_fluid_integrate( N_FLUID *fluid ) 
{
    __n_assert( fluid , return FALSE );

    size_t n = fluid -> numY;
    for( size_t i = 1 ; i < fluid -> numX ; i++ )
    {
        for( size_t j = 1; j < fluid -> numY - 1 ; j++ )
        {
            if( fluid -> s[ i*n + j ] != 0.0 && fluid ->s [ i*n + j-1 ] != 0.0 )
                fluid -> v[ i*n + j ] += fluid -> gravity * fluid -> dt ;
        }	 
    }
    return TRUE ;
}

int n_fluid_solveIncompressibility( N_FLUID *fluid )
{
    __n_assert( fluid , return FALSE );
    size_t n  = fluid -> numY ;
    double cp = fluid -> density / fluid -> dt ;

    for( size_t iter = 0 ; iter < fluid -> numIters ; iter++ )
    {
        for( size_t i = 1 ; i < fluid -> numX - 1 ; i++ ) 
        {
            for( size_t j = 1 ; j < fluid -> numY - 1 ; j++ ) 
            {
                if( fluid ->s[ i*n + j ] == 0.0 )
                    continue;

                double sx0 = fluid ->s[ (i - 1)*n + j ];
                double sx1 = fluid ->s[ (i + 1)*n + j ];
                double sy0 = fluid ->s[ i*n + j - 1 ];
                double sy1 = fluid ->s[ i*n + j + 1 ];
                double s = sx0 + sx1 + sy0 + sy1;
                if( s == 0.0 )
                    continue;

                double div = fluid ->u[(i+1)*n + j] - fluid ->u[i*n + j] + fluid ->v[i*n + j+1] - fluid ->v[i*n + j];
                double p = -div / s ;
                p *= fluid -> overRelaxation ;
                fluid ->p[i*n + j] += cp * p;
                fluid ->u[i*n + j] -= sx0 * p;
                fluid ->u[(i+1)*n + j] += sx1 * p;
                fluid ->v[i*n + j] -= sy0 * p;
                fluid ->v[i*n + j+1] += sy1 * p;
            }
        }
    }
    return TRUE ;
}

int n_fluid_extrapolate( N_FLUID *fluid )
{
    __n_assert( fluid , return FALSE );
    size_t n = fluid -> numY ;
    for( size_t i = 0; i < fluid ->numX ; i++ )
    {
        fluid ->u[ i*n + 0 ] = fluid ->u[i*n + 1];
        fluid ->u[ i*n + fluid ->numY-1 ] = fluid ->u[i*n + fluid ->numY-2]; 
    }
    for( size_t j = 0; j < fluid ->numY; j++) 
    {
        fluid ->v[0*n + j] = fluid ->v[1*n + j];
        fluid ->v[(fluid ->numX-1)*n + j] = fluid ->v[(fluid ->numX-2)*n + j] ;
    }
    return TRUE ;
}

double n_fluid_sampleField( N_FLUID *fluid , double x , double y , uint32_t field )
{
    __n_assert( fluid , return FALSE );
    size_t n = fluid -> numY;
    double h1 = 1.0 ;
    double h2 = 0.5 ;

    x = MIN( x , fluid ->numX );
    y = MIN( y , fluid ->numY );

    double dx = 0.0;
    double dy = 0.0;

    double *f = NULL ;
    switch( field )
    {
        case N_FLUID_U_FIELD: f = fluid ->u ; dy = h2 ; break ;
        case N_FLUID_V_FIELD: f = fluid ->v ; dx = h2 ; break ;
        case N_FLUID_S_FIELD: f = fluid ->m ; dx = h2 ; dy = h2 ; break ;
    }

    double x0 = MIN( floor( (x-dx) * h1 ) , fluid ->numX - 1 );
    double tx = ((x-dx) - x0 ) * h1;
    double x1 = MIN(x0 + 1, fluid ->numX-1);

    double y0 = MIN( floor( (y-dy)*h1 ) , fluid ->numY - 1 );
    double ty = ((y-dy) - y0) * h1;
    double y1 = MIN( y0 + 1 , fluid ->numY-1 );

    double sx = 1.0 - tx;
    double sy = 1.0 - ty;

    double val = sx*sy * f[(size_t)(x0*n + y0)] +
        tx*sy * f[(size_t)(x1*n + y0)] +
        tx*ty * f[(size_t)(x1*n + y1)] +
        sx*ty * f[(size_t)(x0*n + y1)];

    return val;
}

double n_fluid_avgU( N_FLUID *fluid , size_t i , size_t j )
{
    __n_assert( fluid , return FALSE );
    size_t n = fluid ->numY ;
    double u = (fluid ->u[i*n + j-1] + fluid ->u[i*n + j] +
            fluid ->u[(i+1)*n + j-1] + fluid ->u[(i+1)*n + j]) * 0.25;

    return u;

}

double n_fluid_avgV( N_FLUID *fluid , size_t i , size_t j )
{
    __n_assert( fluid , return FALSE );
    size_t n = fluid ->numY;
    double v = (fluid ->v[(i-1)*n + j] + fluid ->v[i*n + j] +
            fluid ->v[(i-1)*n + j+1] + fluid ->v[i*n + j+1]) * 0.25;
    return v;
}

int n_fluid_advectVel( N_FLUID *fluid )
{
    __n_assert( fluid , return FALSE );
    memcpy( fluid -> newU , fluid -> u , fluid -> numCells * sizeof( double ) );
    memcpy( fluid -> newV , fluid -> v , fluid -> numCells * sizeof( double ) );
    size_t n = fluid ->numY;
    double h2 = 0.5 ;
    for( size_t i = 1; i < fluid -> numX ; i++ )
    {
        for( size_t j = 1; j < fluid -> numY ; j++ )
        {
            // u component
            if(fluid -> s[i*n + j] != 0.0 && fluid -> s[(i-1)*n + j] != 0.0 && j < fluid -> numY - 1)
            {
                double x = i;
                double y = j + h2;
                double u = fluid -> u[i*n + j];
                double v = n_fluid_avgV( fluid , i , j );
                //double v = n_fluid_sampleField( fluid , x , y , N_FLUID_V_FIELD );
                x = x - fluid -> dt*u;
                y = y - fluid -> dt*v;
                u = n_fluid_sampleField( fluid , x , y , N_FLUID_U_FIELD );
                fluid ->newU[i*n + j] = u;
            }
            // v component
            if (fluid ->s[i*n + j] != 0.0 && fluid ->s[i*n + j-1] != 0.0 && i < fluid ->numX - 1) {
                double x = i + h2;
                double y = j;
                double u = n_fluid_avgU( fluid , i, j );
                // double u = n_fluid_sampleField( fluid , x , y , N_FLUID_U_FIELD );
                double v = fluid ->v[i*n + j];
                x = x - fluid -> dt*u;
                y = y - fluid -> dt*v;
                v = n_fluid_sampleField( fluid , x , y , N_FLUID_V_FIELD );
                fluid ->newV[i*n + j] = v;
            }
        }	 
    }
    memcpy( fluid -> u , fluid -> newU , fluid -> numCells * sizeof( double ) );
    memcpy( fluid -> v , fluid -> newV , fluid -> numCells * sizeof( double ) );
    return TRUE ;
}

int n_fluid_advectSmoke( N_FLUID *fluid )
{
    __n_assert( fluid , return FALSE );

    size_t n = fluid ->numY;
    double h2 = 0.5 ;

    memcpy( fluid -> newM , fluid -> m , fluid -> numCells * sizeof( double ) );

    for( size_t i = 1; i < fluid -> numX - 1 ; i++ )
    {
        for( size_t j = 1; j < fluid -> numY - 1 ; j++ )
        {
            if( fluid -> s[i*n + j] != 0.0 )
            {
                double u = (fluid ->u[i*n + j] + fluid ->u[(i+1)*n + j]) * 0.5;
                double v = (fluid ->v[i*n + j] + fluid ->v[i*n + j+1]) * 0.5;
                double x = i + h2 - fluid -> dt*u;
                double y = j + h2 - fluid -> dt*v;

                fluid ->newM[i*n + j] = n_fluid_sampleField( fluid , x , y , N_FLUID_S_FIELD );
            }
        }	 
    }

    memcpy( fluid -> m , fluid -> newM , fluid -> numCells * sizeof( double ) );
    
    return TRUE ;
}

int n_fluid_simulate( N_FLUID *fluid )
{
    __n_assert( fluid , return FALSE );
    n_fluid_integrate( fluid );
    memset( fluid -> p , 0 , fluid -> numCells * sizeof( double ) );
    n_fluid_solveIncompressibility( fluid );
    n_fluid_extrapolate( fluid );
    n_fluid_advectVel( fluid );
    n_fluid_advectSmoke( fluid );
    return TRUE ;
}

int n_fluid_set_params( N_FLUID *fluid , double gravity , double dt , size_t numIters , bool overRelaxation )
{
    __n_assert( fluid , return FALSE );
    fluid -> gravity = gravity ;
    fluid -> dt = dt ;
    fluid -> numIters = numIters ;
    fluid -> overRelaxation = overRelaxation ;
    return TRUE ;
}

int n_fluid_setObstacle( N_FLUID *fluid , double x , double y , double vx , double vy , double r , bool reset )
{
    __n_assert( fluid , return FALSE );

    size_t n = fluid -> numY;
    for( size_t i = 1; i < fluid -> numX - 2 ; i++ )
    {
        for( size_t j = 1 ; j < fluid -> numY - 2 ; j++ )
        {

            if( reset )
			{
				fluid -> s[i*n + j] = 1.0;
			}
			
            double dx = (i + 0.5) - x;
            double dy = (j + 0.5) - y;

            if( i > 7 && ( dx * dx + dy * dy < r * r) )
            {
                fluid -> s[i*n + j] = 0.0;
                fluid -> m[i*n + j] = 1.0;
                fluid -> u[i*n + j] = vx;
                fluid -> u[(i+1)*n + j] = vx;
                fluid -> v[i*n + j] = vy;
                fluid -> v[i*n + j+1] = vy;
            }
        }
    }
    return TRUE ;
}


int n_fluid_apply_obstacle_list( N_FLUID *fluid )
{
    __n_assert( fluid , return FALSE );
    __n_assert( fluid , return FALSE );

    return TRUE ;
}


ALLEGRO_COLOR n_fluid_getSciColor( double val , double minVal , double maxVal )
{
    val = MIN( MAX( val , minVal ) , maxVal - 0.0001 );
    double d = maxVal - minVal;
    val = d == 0.0 ? 0.5 : (val - minVal) / d;
    double m = 0.25;
    size_t num = floor( val / m );
    double s = (val - num * m) / m;
    double r = 0.0 , g = 0.0 , b = 0.0 ;
    switch (num) {
        case 0 : r = 0.0; g = s; b = 1.0; break;
        case 1 : r = 0.0; g = 1.0; b = 1.0-s; break;
        case 2 : r = s; g = 1.0; b = 0.0; break;
        case 3 : r = 1.0; g = 1.0 - s; b = 0.0; break;
    }
    //return[255*r,255*g,255*b, 255]
    return al_map_rgb_f( r , g , b );
}

int n_fluid_draw( N_FLUID *fluid , double cScale ) 
{
    __n_assert( fluid , return FALSE );

    size_t n = fluid -> numY ;

    double minP = fluid -> p[0];
    double maxP = fluid -> p[0];

    /*for( size_t i = 0; i < fluid -> numCells; i++)
    {
        minP = MIN( minP , fluid -> p[i] );
        maxP = MAX( maxP , fluid -> p[i] );
    }*/

    ALLEGRO_COLOR color = al_map_rgb( 255, 255, 255 );

    for (size_t i = 0; i < fluid -> numX; i++) 
    {
        for (size_t j = 0; j < fluid -> numY; j++) 
        {
            int64_t x = i * cScale ;
            int64_t y = j * cScale ;
            int64_t cx = x + cScale ; 
            int64_t cy = y + cScale ;

            //double p = fluid -> p[i*n + j];
            //double s = fluid -> m[i*n + j];
            //color = n_fluid_getSciColor( p , minP , maxP );

            double s = fluid -> m[i*n + j];
            al_draw_filled_rectangle( x , y , cx , cy , al_map_rgb_f( 1.0 - s , 0 , 0 ) );
        }
    }
    return TRUE ;
}
