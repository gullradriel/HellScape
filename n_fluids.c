#include "n_fluids.h"
#include "math.h"

#define N_FLUID_U_FIELD = 0;
#define N_FLUID V_FIELD = 1;
#define N_FLUID S_FIELD = 2;

void n_memset( void *dst , void *val , size_t size , size_t count )
{
    void *ptr = dst;
    while( count-- > 0 )
    {
        memcpy( ptr , val , size );
        ptr += size;
    }
    return dst;
}

int destroy_n_fluid( N_EULER_SIMULATION **fluid_sim )
{
    __n_assert( (*fluid_sim) , return FALSE );

    FreeNoLog( fluid_sim -> u );
    FreeNoLog( fluid_sim -> newU );
    FreeNoLog( fluid_sim -> v );
    FreeNoLog( fluid_sim -> newV );
    FreeNoLog( fluid_sim -> p );
    FreeNoLog( fluid_sim -> s );
    FreeNoLog( fluid_sim -> m );
    FreeNoLog( fluid_sim -> newM );
    FreeNoLog( (*fluid_sim ) );

    return TRUE ;
}

N_EULER_SIMULATION *new_n_fluid( double density , size_t sx , size_t sy , double h )
{
    N_EULER_SIMULATION *fluid_sim = NULL ;

    Malloc( fluid_sim , N_EULER_SIMULATION , 1 );
    __n_assert( fluid_sim , return NULL );

    fluid_sim -> density = density ;
    fluid_sim -> numX = sx + 2 ;
    fluid_sim -> numY = sy + 2 ;
    fluid_sim -> numCells = fluid_sim -> numX * fluid_sim -> numY ;
    fluid_sim -> gravity = -9.81 ;
    fluid_sim -> nIters = 100 ;
    fluid_sim -> h = h ;

    Malloc( fluid_sim -> u    , double , fluid_sim -> numCells );
    Malloc( fluid_sim -> newU , double , fluid_sim -> numCells );
    Malloc( fluid_sim -> v    , double , fluid_sim -> numCells );
    Malloc( fluid_sim -> newV , double , fluid_sim -> numCells );
    Malloc( fluid_sim -> p    , double , fluid_sim -> numCells );
    Malloc( fluid_sim -> s    , double , fluid_sim -> numCells );
    Malloc( fluid_sim -> m    , double , fluid_sim -> numCells );
    Malloc( fluid_sim -> newM , double , fluid_sim -> numCells );

    double d_val = 1.0 ;
    n_memset( fluid_sim -> m , &d_val , sizeof( d_val ) , fluid_sim -> numCells );

    return fluid_sim ;
} /* new_n_fluid */


int n_fluid_integrate( N_EULER_SIMULATION *fluid_sim ) 
{
    __n_assert( fluid_sim , return FALSE );

    size_t n = fluid_sim -> numY;
    for( size_t i = 1 ; i < fluid_sim -> numX ; i++ )
    {
        for( size_t j = 1; j < fluid_sim -> numY - 1 ; j++ )
        {
            if( fluid_sim -> s[ i*n + j ] != 0.0 && fluid_sim ->s [ i*n + j-1 ] != 0.0 )
                fluid_sim -> v[ i*n + j ] += fluid_sim -> gravity * fluid_sim -> dt ;
        }	 
    }
    return TRUE ;
}

int n_fluid_solveIncompressibility( N_EULER_SIMULATION *fluid_sim )
{
    __n_assert( fluid_sim , return FALSE );
    size_t n  = fluid_sim -> numY ;
    double cp = ( fluid_sim -> density * fluid_sim -> h ) / fluid_sim -> dt ;

    for( size_t iter = 0 ; iter < numIters ; iter++ )
    {
        for( size_t i = 1 ; i < fluid_sim -> numX - 1 ; i++ ) 
        {
            for( size_t j = 1 ; j < fluid_sim -> numY - 1 ; j++ ) 
            {
                if( fluid_sim ->s[ i*n + j ] == 0.0 )
                    continue;

                double s = fluid_sim ->s[ i*n + j ];
                double sx0 = fluid_sim ->s[ (i - 1)*n + j ];
                double sx1 = fluid_sim ->s[ (i + 1)*n + j ];
                double sy0 = fluid_sim ->s[ i*n + j - 1 ];
                double sy1 = fluid_sim ->s[ i*n + j + 1 ];
                double s = sx0 + sx1 + sy0 + sy1;
                if( s == 0.0 )
                    continue;

                double div = fluid_sim ->u[(i+1)*n + j] - fluid_sim ->u[i*n + j] + fluid_sim ->v[i*n + j+1] - fluid_sim ->v[i*n + j];
                double p = -div / s ;
                p *= fluid_sim -> overRelaxation ;
                fluid_sim ->p[i*n + j] += cp * p;
                fluid_sim ->u[i*n + j] -= sx0 * p;
                fluid_sim ->u[(i+1)*n + j] += sx1 * p;
                fluid_sim ->v[i*n + j] -= sy0 * p;
                fluid_sim ->v[i*n + j+1] += sy1 * p;
            }
        }
    }
    return TRUE ;
}

int n_fluid_extrapolate( N_EULER_SIMULATION *fluid_sim )
{
    __n_assert( fluid_sim , return FALSE );
    double n = fluid_sim -> numY ;
    for( size_t i = 0; i < fluid_sim ->numX ; i++ )
    {
        fluid_sim ->u[ i*n + 0 ] = fluid_sim ->u[i*n + 1];
        fluid_sim ->u[ i*n + fluid_sim ->numY-1 ] = fluid_sim ->u[i*n + fluid_sim ->numY-2]; 
    }
    for( size_t j = 0; j < fluid_sim ->numY; j++) 
    {
        fluid_sim ->v[0*n + j] = fluid_sim ->v[1*n + j];
        fluid_sim ->v[(fluid_sim ->numX-1)*n + j] = fluid_sim ->v[(fluid_sim ->numX-2)*n + j] 
    }
    return TRUE ;
}

double n_fluid_sampleField( N_EULER_SIMULATION *fluid_sim , size_t x , size_t y , uint32_t field )
{
    __n_assert( fluid_sim , return FALSE );
    size_t n = fluid_sim -> numY;
    double h = fluid_sim -> h ;
    double h1 = 1.0 / h;
    double h2 = 0.5 * h;

    x = Max( Min( x , fluid_sim ->numX * fluid_sim -> h ) , fluid_sim -> h);
    y = Max( Min( y , fluid_sim ->numY * fluid_sim -> h ) , fluid_sim -> h);

    double dx = 0.0;
    double dy = 0.0;

    double f;
    switch( field )
    {
        case N_FLUID_U_FIELD: f = fluid_sim ->u ; dy = h2 ; break ;
        case N_FLUID_V_FIELD: f = fluid_sim ->v ; dx = h2 ; break ;
        case N_FLUID_S_FIELD: f = fluid_sim ->m ; dx = h2 ; dy = h2 ; break
    }

    double x0 = Min( floor( (x-dx) * h1 ) , fluid_sim ->numX - 1 );
    double tx = ((x-dx) - x0*h) * h1;
    double x1 = Min(x0 + 1, fluid_sim ->numX-1);

    double y0 = Min( floor( (y-dy)*h1 ) , fluid_sim ->numY - 1 );
    double ty = ((y-dy) - y0*h) * h1;
    double y1 = Min( y0 + 1 , fluid_sim ->numY-1 );

    double sx = 1.0 - tx;
    double sy = 1.0 - ty;

    double val = sx*sy * f[x0*n + y0] +
        tx*sy * f[x1*n + y0] +
        tx*ty * f[x1*n + y1] +
        sx*ty * f[x0*n + y1];

    return val;
}

double n_fluid_avgU( N_EULER_SIMULATION *fluid_sim , size_t i , size_t j )
{
    __n_assert( fluid_sim , return FALSE );
    double n = fluid_sim ->numY ;
    double u = (fluid_sim ->u[i*n + j-1] + fluid_sim ->u[i*n + j] +
            fluid_sim ->u[(i+1)*n + j-1] + fluid_sim ->u[(i+1)*n + j]) * 0.25;

    return u;

}

double n_fluid_avgV( N_EULER_SIMULATION *fluid_sim , size_t i , size_t j )
{
    __n_assert( fluid_sim , return FALSE );
    double n = fluid_sim ->numY;
    double v = (fluid_sim ->v[(i-1)*n + j] + fluid_sim ->v[i*n + j] +
            fluid_sim ->v[(i-1)*n + j+1] + fluid_sim ->v[i*n + j+1]) * 0.25;
    return v;
}

int n_fluid_advectVel( N_EULER_SIMULATION *fluid_sim )
{
    __n_assert( fluid_sim , return FALSE );
    memcpy( fluid_sim -> newU , fluid_sim -> u , sizeof( fluid_sim -> u ) );
    memcpy( fluid_sim -> newV , fluid_sim -> v , sizeof( fluid_sim -> u ) );
    size_t n = fluid_sim ->numY;
    double h = fluid_sim ->h;
    double h2 = 0.5 * h;
    for( size_t i = 1; i < fluid_sim -> numX ; i++ )
    {
        for( size_t j = 1; j < fluid_sim -> numY ; j++ )
        {
            // u component
            if(fluid_sim -> s[i*n + j] != 0.0 && fluid_sim -> s[(i-1)*n + j] != 0.0 && j < fluid_sim -> numY - 1)
            {
                double x = i*h;
                double y = j*h + h2;
                double u = fluid_sim -> u[i*n + j];
                double v = n_fluid_avgV( fluid_sim , i , j );

                //						var v = n_fluid_sampleField( fluid_sim , x , y , N_FLUID_V_FIELD );
                x = x - fluid_sim -> dt*u;
                y = y - fluid_sim -> dt*v;
                u = n_fluid_sampleField( field_sim , x , y , N_FLUID_U_FIELD );
                fluid_sim ->newU[i*n + j] = u;
            }
            // v component
            if (fluid_sim ->s[i*n + j] != 0.0 && fluid_sim ->s[i*n + j-1] != 0.0 && i < fluid_sim ->numX - 1) {
                var x = i*h + h2;
                var y = j*h;
                var u = fluid_sim ->avgU(i, j);
                //						var u = n_fluid_sampleField( fluid_sim , x , y , N_FLUID_U_FIELD );
                var v = fluid_sim ->v[i*n + j];
                x = x - dt*u;
                y = y - dt*v;
                v = n_fluid_sampleField( field_sim , x , y , N_FLUID_V_FIELD );
                fluid_sim ->newV[i*n + j] = v;
            }
        }	 
    }
    memcpy( fluid_sim -> u , fluid_sim -> newU , sizeof( fluid_sim -> newU ) );
    memcpy( fluid_sim -> v , fluid_sim -> newU , sizeof( fluid_sim -> newv ) );
    return TRUE ;
}

int n_fluid_advectSmoke( N_EULER_SIMULATION *fluid_sim )
{
    __n_assert( fluid_sim , return FALSE );

    memcpy( fluid_sim -> newM , fluid_sim -> m , sizeof( fluid_sim -> m ) );

    size_t n = fluid_sim ->numY;
    double h = fluid_sim ->h;
    double h2 = 0.5 * h;

    for( size_t i = 1; i < fluid_sim -> numX - 1 ; i++ )
    {
        for( size_t j = 1; j < fluid_sim -> numY - 1 ; j++ )
        {
            if( fluid_sim -> s[i*n + j] != 0.0 )
            {
                double u = (fluid_sim ->u[i*n + j] + fluid_sim ->u[(i+1)*n + j]) * 0.5;
                double v = (fluid_sim ->v[i*n + j] + fluid_sim ->v[i*n + j+1]) * 0.5;
                double x = i*h + h2 - dt*u;
                double y = j*h + h2 - dt*v;

                fluid_sim ->newM[i*n + j] = n_fluid_sampleField( field_sim , x , y , N_FLUID_S_FIELD );
            }
        }	 
    }
    memcpy( fluid_sim -> m , fluid_sim -> newM , sizeof( fluid_sim -> newM ) );
    return TRUE ;
}

int n_fluid_simulate( N_EULER_SIMULATION *fluid_sim )
{
    __n_assert( fluid_sim , return FALSE );
    n_fluid_integrate( fluid_sim );
    memset( fluid_sim -> p , 0 , sizeof( p ) );
    n_fluid_solveIncompressibility( fluid_sim );
    n_fluid_extrapolate( fluid_sim );
    n_fluid_advectVel( fluid_sim );
    n_fluid_advectSmoke( fluid_sim );
    return TRUE ;
}

int n_fluid_set_params( N_EULER_SIMULATION *fluid_sim , double gravity , double dt , size_t numIters , bool overRelaxation )
{
    __n_assert( fluid_sim , return FALSE );
    fluid_sim -> gravity = gravity ;
    fluid_sim -> gravity = dt ;
    fluid_sim -> gravity = numIters ;
    fluid_sim -> gravity = overRelaxation ;
    return TRUE ;
}

int n_fluid_apply_obstacle_list( N_EULER_SIMULATION *fluid_sim )
{
    __n_assert( fluid_sim , return FALSE );

    return TRUE ;
}

int n_fluid_setObstacle( N_EULER_SIMULATION *fluid_sim , double x , double y , double r , bool reset )
{
    __n_assert( fluid_sim , return FALSE );

    double vx = 0.0;
    double vy = 0.0;

    if( !reset ) 
    {
        vx = (x - field_sim -> obstacleX) / field_sim -> dt;
        vy = (y - field_sim -> obstacleY) / field_sim -> dt;
    }

    field_sim -> obstacleX = x;
    field_sim -> obstacleY = y;
    size_t n = fluid_sim -> numY;
    double cd = sqrt( 2 ) * fluid_sim -> h;

    for( size_t i = 1; i < field_sim -> numX - 2 ; i++ )
    {
        for( size_t j = 1 ; j < field_sim -> numY - 2 ; j++ )
        {

            field_sim -> s[i*n + j] = 1.0;

            dx = (i + 0.5) * field_sim -> h - x;
            dy = (j + 0.5) * field_sim -> h - y;

            if (dx * dx + dy * dy < r * r) 
            {
                field_sim -> s[i*n + j] = 0.0;

                if (field_sim -> sceneNr == 2) 
                    field_sim -> m[i*n + j] = 0.5 + 0.5 * Math.sin(0.1 * field_sim -> frameNr)
                else 
                    field_sim -> m[i*n + j] = 1.0;

                field_sim -> u[i*n + j] = vx;
                field_sim -> u[(i+1)*n + j] = vx;
                field_sim -> v[i*n + j] = vy;
                field_sim -> v[i*n + j+1] = vy;
            }
        }
    }
    return TRUE ;
}

N_FLUID *n_fluid_setupScene() 
{
    fluid_sim -> overRelaxation = 1.9;
    field_sim -> dt = 1.0 / 60.0;
    field_sim -> numIters = 40;

    double res = 100;
    double domainHeight = 1.0;
    double domainWidth = domainHeight / simHeight * simWidth;
    double h = domainHeight / res;

    double numX = floor( domainWidth / h );
    double numY = floor( domainHeight / h );

    double density = 1000.0;

    N_FLUID *field_sim = new_n_fluid( density , numX , numY , h );
    size_t n = field_sim -> numY;
    double inVel = 2.0;
    for( size_t i = 0; i < field_sim -> numX; i++ ) 
    {
        for( size_t j = 0; j < field_sim -> numY; j++ ) 
        {
            double s = 1.0;	// fluid
            if( i == 0 || j == 0 || j == field_sim -> numY -  1 )
                s = 0.0;	// solid
            field_sim -> s[i*n + j] = s

            if( i == 1 )
            {
                field_sim -> u[i*n + j] = inVel;
            }
        }
    }

    double pipeH = 0.1 * field_sim -> numY ;
    double minJ = floor( 0.5 * field_sim -> numY - 0.5*pipeH );
    double maxJ = floor( 0.5 * field_sim -> numY + 0.5*pipeH );

    for( size_t j = minJ ; j < maxJ ; j++ )
        field_sim -> m[j] = 0.0;

    setObstacle(0.4, 0.5, true)
    n_fluid_setObstacle( fluid_sim , 0.4 , 0.5 , 0.15 , true )
    field_sim -> gravity = 0.0;
    return field_sim ;
}


// draw -------------------------------------------------------

function setColor(r,g,b) {
    c.fillStyle = `rgb(
            ${Math.floor(255*r)},
            ${Math.floor(255*g)},
            ${Math.floor(255*b)})`
        c.strokeStyle = `rgb(
                ${Math.floor(255*r)},
                ${Math.floor(255*g)},
                ${Math.floor(255*b)})`
}

function getSciColor(val, minVal, maxVal) {
    val = Math.min(Math.max(val, minVal), maxVal- 0.0001);
    var d = maxVal - minVal;
    val = d == 0.0 ? 0.5 : (val - minVal) / d;
    var m = 0.25;
    var num = Math.floor(val / m);
    var s = (val - num * m) / m;
    var r, g, b;

    switch (num) {
        case 0 : r = 0.0; g = s; b = 1.0; break;
        case 1 : r = 0.0; g = 1.0; b = 1.0-s; break;
        case 2 : r = s; g = 1.0; b = 0.0; break;
        case 3 : r = 1.0; g = 1.0 - s; b = 0.0; break;
    }

    return[255*r,255*g,255*b, 255]
}

function draw() 
{
    c.clearRect(0, 0, canvas.width, canvas.height);

    c.fillStyle = "#FF0000";
    f = field_sim -> fluid;
    n = field_sim -> numY;

    var cellScale = 1.1;

    var h = field_sim -> h;

    minP = field_sim -> p[0];
    maxP = field_sim -> p[0];

    for (var i = 0; i < field_sim -> numCells; i++) {
        minP = Math.min(minP, field_sim -> p[i]);
        maxP = Math.max(maxP, field_sim -> p[i]);
    }

    id = c.getImageData(0,0, canvas.width, canvas.height)

        var color = [255, 255, 255, 255]

        for (var i = 0; i < field_sim -> numX; i++) {
            for (var j = 0; j < field_sim -> numY; j++) {

                if (field_sim -> showPressure) {
                    var p = field_sim -> p[i*n + j];
                    var s = field_sim -> m[i*n + j];
                    color = getSciColor(p, minP, maxP);
                    if (field_sim -> showSmoke) {
                        color[0] = Math.max(0.0, color[0] - 255*s);
                        color[1] = Math.max(0.0, color[1] - 255*s);
                        color[2] = Math.max(0.0, color[2] - 255*s);
                    }
                }
                else if (field_sim -> showSmoke) {
                    var s = field_sim -> m[i*n + j];
                    color[0] = 255*s;
                    color[1] = 255*s;
                    color[2] = 255*s;
                    if (field_sim -> sceneNr == 2)
                        color = getSciColor(s, 0.0, 1.0);
                }
                else if (field_sim -> s[i*n + j] == 0.0) {
                    color[0] = 0;
                    color[1] = 0;
                    color[2] = 0;
                }

                var x = Math.floor(cX(i * h));
                var y = Math.floor(cY((j+1) * h));
                var cx = Math.floor(cScale * cellScale * h) + 1;
                var cy = Math.floor(cScale * cellScale * h) + 1;

                r = color[0];
                g = color[1];
                b = color[2];

                for (var yi = y; yi < y + cy; yi++) {
                    var p = 4 * (yi * canvas.width + x)

                        for (var xi = 0; xi < cx; xi++) {
                            id.data[p++] = r;
                            id.data[p++] = g;
                            id.data[p++] = b;
                            id.data[p++] = 255;
                        }
                }
            }
        }

    c.putImageData(id, 0, 0);

    if (field_sim -> showVelocities) {

        c.strokeStyle = "#000000";	
        scale = 0.02;	

        for (var i = 0; i < field_sim -> numX; i++) {
            for (var j = 0; j < field_sim -> numY; j++) {

                var u = field_sim -> u[i*n + j];
                var v = field_sim -> v[i*n + j];

                c.beginPath();

                x0 = cX(i * h);
                x1 = cX(i * h + u * scale);
                y = cY((j + 0.5) * h );

                c.moveTo(x0, y);
                c.lineTo(x1, y);
                c.stroke();

                x = cX((i + 0.5) * h);
                y0 = cY(j * h );
                y1 = cY(j * h + v * scale)

                    c.beginPath();
                c.moveTo(x, y0);
                c.lineTo(x, y1);
                c.stroke();

            }
        }
    }

    if (field_sim -> showObstacle) {

        c.strokeW
            r = field_sim -> obstacleRadius + field_sim -> h;
        if (field_sim -> showPressure)
            c.fillStyle = "#000000";
        else
            c.fillStyle = "#DDDDDD";
        c.beginPath();	
        c.arc(
                cX(field_sim -> obstacleX), cY(field_sim -> obstacleY), cScale * r, 0.0, 2.0 * Math.PI); 
        c.closePath();
        c.fill();

        c.lineWidth = 3.0;
        c.strokeStyle = "#000000";
        c.beginPath();	
        c.arc(
                cX(field_sim -> obstacleX), cY(field_sim -> obstacleY), cScale * r, 0.0, 2.0 * Math.PI); 
        c.closePath();
        c.stroke();
        c.lineWidth = 1.0;
    }
}

// main -------------------------------------------------------

function simulate() 
{
    if (!field_sim -> paused)
        field_sim -> fluid.simulate(field_sim -> dt, field_sim -> gravity, field_sim -> numIters)
            field_sim -> frameNr++;
}

function update() {
    simulate();
    draw();
    requestAnimationFrame(update);
}

setupScene(1);
update();
