typedef struct N_EULER_SIMULATION
{
    size_t x_size ;
    size_t y_size ;
    size_t z_size ;

    double h ;
    double density ;
    double sim_height ;
    double sim_width ;
    double c_scale ;

    double *u ;
    double *new_u ;

    double *v ;
    double *new_v ;
    
    double *p ;
    double *s ;

    double *m ;
    double *new_m ;

    double gravity[ 3 ];

} N_EULER_ARRAY ;


