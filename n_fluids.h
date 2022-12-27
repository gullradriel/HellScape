typedef struct N_EULER_SIMULATION
{
    size_t numX ;
    size_t numY ;
    size_t numZ ;
    size_t numCells ;
    size_t nulIters ;
    
    double h ;
    double density ;
    double dt ;
    double gravity ;
    double sim_height ;
    double sim_width ;
    double c_scale ;

    double *u ;
    double *newU ;

    double *v ;
    double *newV ;
    
    double *p ;
    double *s ;

    double *m ;
    double *newM ;

} N_EULER_ARRAY ;


