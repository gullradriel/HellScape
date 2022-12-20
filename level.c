/**\file level.c
 *
 *  level file for hacks
 *
 *\author Castagnier Mickaël aka Gull Ra Driel
 *
 *\version 1.0
 *
 *\date 29/12/2021 based on 30/12/2016
 *
 */

#include "level.h"
#include "ctype.h"
#include "nilorea/n_anim.h"



MONSTER *new_monster( int life, int type, PHYSICS physics )
{
	static int nb = 0 ;

	MONSTER *monster = NULL ;
	Malloc( monster, MONSTER, 1 );
	__n_assert( monster, return NULL );

	memcpy( &monster -> physics, &physics, sizeof( PHYSICS ) );
	memset( &monster -> attr , 0 , sizeof( ATTRIBUTES ) );

	monster -> attr . life = life ;
	monster -> attr . type = type ;
	monster -> attr . xp = 0 ;
	monster -> attr . action = ACTION_MOVE ;
	monster -> physics . sz = 15 ;
	monster -> physics . type = type ;
	nb ++ ;
	return monster;
}



int get_level_data( LEVEL *level, PHYSICS *physics, int mx, int my, int *x, int *y )
{
	int cellx = 0, celly = 0 ;

	cellx = ( physics -> position[ 0 ] + mx ) / level -> tilew ;
	celly = ( physics -> position[ 1 ] + my ) / level -> tileh ;

	if( cellx >= 0 && cellx < level -> w && celly >= 0 && celly < level -> h )
	{
		if( x != NULL )
			(*x) = cellx ;
		if( y != NULL )
			(*y) = celly ;

		return level -> cells[ cellx ][ celly ];
	}
	return -1000 ;
}



int test_coord( LEVEL *level, PHYSICS *physics, VECTOR3D fricton, int off_x, int off_y )
{
	int cellx = 0, celly = 0, done = 0, once = 0, has_blocked = 0 ;

	if( physics -> type == 100 )
		return 0 ;

	/* left */
	once = 0 ;
	do
	{
		int cellstate = get_level_data( level, physics, off_x - physics -> sz, off_y, &cellx, &celly );

		if( cellstate != -1000 )
		{
			if( ( physics -> type == 2 && cellstate == BLOCKING_CELL ) || ( physics -> type != 2  && cellstate < PASSING_CELL_1 ) )
			{
				physics -> position[ 0 ] = physics -> position[ 0 ] + 1.0 ;
				if( !once )
				{
					once = has_blocked = 1 ;
					physics -> speed[ 0 ] = fabs(  fabs( physics -> speed[ 0 ] ) - fabs( fricton[ 0 ] ) );
					if( physics -> speed[ 0 ] < 0.0 )
						physics -> speed[ 0 ] = 0.0 ;
					if( level -> gravity[ 1 ] > 0 )
						physics -> speed[ 0 ] = 0.0 ;
				}
				done = 0 ;
			}
			else
				done = 1 ;
		}
		else
			done = 1 ;
	}
	while( !done );

	/* right */
	once = 0 ;
	do
	{
		int cellstate = get_level_data( level, physics, off_x + physics -> sz, off_y, &cellx, &celly );

		if( cellstate != -1000 )
		{
			if( ( physics -> type == 2 && cellstate == BLOCKING_CELL ) || ( physics -> type != 2  && cellstate < PASSING_CELL_1 ) )
			{
				physics -> position[ 0 ] = physics -> position[ 0 ] - 1.0 ;
				if( !once )
				{
					once = has_blocked = 1 ;
					physics -> speed[ 0 ] = - fabs(  fabs(physics -> speed[ 0 ]) -  fabs(fricton[ 0 ]) );
					if( physics -> speed[ 0 ] > 0.0 )
						physics -> speed[ 0 ] = 0.0 ;
					if( level -> gravity[ 1 ] > 0 )
						physics -> speed[ 0 ] = 0.0 ;
				}
				done = 0 ;
			}
			else
				done = 1 ;
		}
		else
			done = 1 ;
	}
	while( !done );

	/* down */
	once = 0 ;
	do
	{
		int cellstate = get_level_data( level, physics, off_x, off_y + physics -> sz, &cellx, &celly );

		if( cellstate != -1000 )
		{
			if( ( physics -> type == 2 && cellstate == BLOCKING_CELL ) || ( physics -> type != 2  && cellstate < PASSING_CELL_1 ) )
			{
				physics -> position[ 1 ] = physics -> position[ 1 ] - 1.0 ;
				if( !once )
				{
					once = has_blocked = 1 ;
					physics -> speed[ 1 ] = -fabs( fabs( physics -> speed[ 1 ] ) - fabs(fricton[ 1 ] ) );
					if( physics -> speed[ 1 ] > 0.0 )
						physics -> speed[ 1 ] = 0.0 ;
					physics -> can_jump = 1 ;
					if( level -> gravity[ 1 ] > 0 )
						physics -> speed[ 1 ] = 0.0 ;
				}
				done = 0 ;
			}
			else
				done = 1 ;
		}
		else
			done = 1 ;
	}
	while( !done );

	/* up */
	once = 0 ;
	do
	{
		int cellstate = get_level_data( level, physics,  off_x, off_y - physics -> sz, &cellx, &celly );

		if( cellstate != -1000 )
		{
			if( ( physics -> type == 2 && cellstate == BLOCKING_CELL ) || ( physics -> type != 2  && cellstate < PASSING_CELL_1 ) )
			{
				physics -> position[ 1 ] = physics -> position[ 1 ] + 1.0 ;
				if( !once )
				{
					once = has_blocked = 1 ;
					physics -> speed[ 1 ] = fabs( fabs(physics -> speed[ 1 ] ) - fabs( fricton[ 1 ] ) );
					if( physics -> speed[ 1 ] < 0.0 )
						physics -> speed[ 1 ] = 0.0 ;
					if( level -> gravity[ 1 ] > 0 )
						physics -> speed[ 1 ] = 0.0 ;
				}
				done = 0 ;
			}
			else
				done = 1 ;
		}
		else
			done = 1 ;
	}
	while( !done );

	if( !has_blocked )
	{
		physics -> can_jump = 0 ;
	}

	return has_blocked ;
}



int animate_physics( LEVEL *level, PHYSICS *physics, VECTOR3D friction, double delta_t )
{
	VECTOR3D old_pos, new_pos, pos_delta ;

	memcpy( &old_pos, &physics -> position, sizeof( VECTOR3D ) );

	update_physics_position( physics, delta_t );

	memcpy( &new_pos, &physics -> position, sizeof( VECTOR3D ) );

	double d = distance ( &old_pos, &physics -> position );

	if( physics -> sz < 1 )
		physics -> sz = 1 ;

	double steps = physics -> sz ;

	if( d >= steps )
	{
		for( int it = 0 ; it < 3 ; it ++ )
			pos_delta[ it ] = ( physics -> position[ it ] - old_pos[ it ] ) / d ;

		double inc = -steps ;

		while( inc < d )
		{
			inc += steps ;
			if( inc > d )
				inc = d ;
			for( int it = 0 ; it < 3 ; it ++ )
				physics -> position[ it ] = old_pos[ it ] + pos_delta[ it ] * inc ;

			if( test_coord( level, physics, friction, 0, 0 ) > 0 )
				break ;
		}
	}
	else
		test_coord( level, physics, friction, 0, 0 );

	return TRUE ;
}

int animate_level( LEVEL *level,  PLAYER *player, double delta_t )
{
	LIST_NODE *node = NULL ;
	MONSTER *monster = NULL ;
	__n_assert( level, return FALSE );
	__n_assert( level -> monster_list, return FALSE );

	PARTICLE *ptr = NULL ;

	node = level -> particle_system_effects -> list -> start ;
	while( node )
	{
		ptr = (PARTICLE *)node -> ptr ;
		if( ptr -> lifetime != -1 )
		{
			ptr -> lifetime -= delta_t/1000.0 ;
			if( ptr -> lifetime == -1 )
				ptr -> lifetime = 0 ;
		}

		if( ptr -> lifetime > 0 || ptr -> lifetime == -1 )
		{
			animate_physics( level, &ptr -> object, level -> friction, delta_t );
			node = node -> next ;
		}
		else
		{
			LIST_NODE *node_to_kill = node ;
			node = node -> next ;
			ptr = remove_list_node( level -> particle_system_effects -> list, node_to_kill, PARTICLE );
			Free( ptr );
		}
	}
	LIST_NODE *bnode = level -> particle_system_bullets -> list -> start ;
	while( bnode )
	{
		PARTICLE *bptr = (PARTICLE *)bnode -> ptr ;
		if( bptr )
		{
			if( bptr -> lifetime != -1 )
			{
				bptr -> lifetime -= delta_t/1000.0 ;
				if( bptr -> lifetime == -1 )
					bptr -> lifetime = 0 ;
			}

			if( bptr -> lifetime > 0 || bptr -> lifetime == -1 )
			{
				node = level -> monster_list -> start ;
				while( node )
				{
					monster= (MONSTER *)node -> ptr ;
					/*x, y, x + 2 * monster -> physics . sz, y - 2 * monster -> physics . sz,*/
					if(      	bptr -> object . position[ 0 ] > monster -> physics . position[ 0 ] &&
							bptr -> object . position[ 1 ] < monster -> physics . position[ 1 ] &&
							bptr -> object . position[ 0 ] < monster -> physics . position[ 0 ] + 2 * monster ->physics . sz &&
							bptr -> object . position[ 1 ] > monster -> physics . position[ 1 ] - 2 * monster ->physics . sz  )
					{
						//bptr -> lifetime = 0 ;
						monster -> attr . life -= bptr -> object . sz ;
						monster -> physics . speed[ 0 ] = monster -> physics . speed[ 1 ] = 0 ;

						PHYSICS tmp_part ;
						tmp_part . sz = 10 ;
						VECTOR3D_SET( tmp_part . speed,
								300 - rand()%600,
								300 - rand()%600,
								0.0  );
						VECTOR3D_SET( tmp_part . position, monster -> physics . position[ 0 ], monster ->  physics . position[ 1 ], 0.0  );
						VECTOR3D_SET( tmp_part . acceleration, 0.0, 0.0, 0.0 );
						VECTOR3D_SET( tmp_part . orientation, 0.0, 0.0, 0.0 );
						add_particle( level -> particle_system_effects, -1, PIXEL_PART, 100 + rand()%500, 1+rand()%3,
								al_map_rgba(   55 + rand()%200,  0, 0, 100 + rand()%155 ), tmp_part );


					}
					node = node -> next ;
				}
				animate_physics( level, &bptr -> object, level -> friction , delta_t );
				bnode = bnode -> next ;
			}
			else
			{
				LIST_NODE *node_to_kill = bnode ;
				bnode = bnode -> next ;
				ptr = remove_list_node( level -> particle_system_bullets -> list, node_to_kill, PARTICLE );
				Free( ptr );
			}
		}
	}


	node = level -> monster_list -> start ;
	while( node )
	{
		monster= (MONSTER *)node -> ptr ;
		if( monster -> attr . life <= 0 )
		{
			if( monster -> attr . type == FINAL_MONSTER )
			{
				(*player) . attr . have_finished = 1000 ;
			}

			(*player) . attr . xp += monster -> attr  . type * 100 ;
			if( (*player) . attr . level > 20 )
			{
				(*player) . attr . level = 20 ;
				(*player) . attr . xp = (*player) . attr . xp_to_level ;
			}
			else
			{
				if( (*player) . attr . xp > (*player) . attr . xp_to_level )
				{
					(*player) . attr . level ++ ;
					(*player) . attr . xp_to_level *= 2 ;
					(*player) . attr . xp = 0 ;
				}
			}


			if( (*player) . attr . have_shield )
			{
				if( rand()%100 < 15 )
				{
					(*player) . attr . nb_shield_potions ++ ;
				}
				else
				{

					if( rand()%100 < 15 ) 
					{
						(*player) . attr . nb_health_potions ++ ;
					}
				}
			}
			else
			{
				if( rand()%100 < 15 ) (*player) . attr . nb_health_potions ++ ;
			}
			LIST_NODE *ntokill = node ;
			if( node -> next )
				node = node -> next ;
			else
				node = NULL ;
			MONSTER *mtokill = remove_list_node( level -> monster_list, ntokill, MONSTER );
			Free( mtokill );
		}
		else
		{


			monster -> physics . can_jump = 0 ;
			animate_physics( level, &monster -> physics, level -> friction , delta_t );

			double itx = 0, ity = 0, itz = 0, sz = 0;
			monster -> attr . next_move_timer -= delta_t ;
			if( monster -> attr . next_move_timer < 0 )
			{
				monster -> attr . action = 0 ;
				int min_attacking_distance = 0 ;
				if( monster -> attr . type == PASSIVE_MONSTER )
				{
					monster -> attr . next_move_timer = 1000000 + rand()%2000000 ;
					min_attacking_distance = 150 ;
				}
				else if( monster -> attr . type == AGRESSIVE_MONSTER )
				{
					monster -> attr . next_move_timer = 1000000 + rand()%1000000 ;
					min_attacking_distance = 300 ;
				}
				else if( monster -> attr . type == BIGBOSS_MONSTER )
				{
					monster -> attr . next_move_timer = 500000 + rand()%500000 ;
					min_attacking_distance = 500 ;
				}
				else if( monster -> attr . type == FINAL_MONSTER )
				{
					monster -> attr . next_move_timer = 500000 + rand()%500000 ;
					min_attacking_distance = 600 ;
				}

				if( distance( &monster -> physics . position , &player -> physics . position ) < min_attacking_distance )
				{

					itx = monster -> physics . position[ 0 ] - (*player) . physics . position[ 0 ] ;
					ity = monster -> physics . position[ 1 ] - (*player) . physics . position[ 1 ] ;
					itz = 0 ;

					sz = sqrt((itx * itx) + (ity * ity) + (itz * itz));
					itx /= sz ;
					ity /= sz ;
					itz /= sz ;
					itx*= 50 ;
					ity*= 50 ;
					itz*= 50 ;
					VECTOR3D_SET( monster -> physics . speed, -itx, -ity, -itz );
					VECTOR3D_SET( monster -> physics . acceleration, -itx, -ity, -itz );
					monster -> attr . action = 1 ;
				}
				else
				{
					if( rand()%100 > 25 )
					{
						VECTOR3D_SET( monster -> physics . speed, 50 - rand()%100 , 50 - rand()%100 , 0 );
						VECTOR3D_SET( monster -> physics . acceleration, 50 - rand()%100 , 50 - rand()%100 , 0 );
					}
					monster -> attr . action = 0 ;
				}
			}

			node = node -> next ;
		}
	}
	return TRUE ;
}



LEVEL *load_level( PLAYER *player , ANIM_LIB *animations , int w, int h )
{
	FILE *in = NULL ;
	LEVEL *lvl = NULL ;

	MONSTER *monster = NULL ;

	in = fopen( _nstr( (*player) . attr . level_string ) , "r" );
	if( !in )
	{
		n_log( LOG_ERR, "%s not found", _nstr( (*player) . attr . level_string ) );
		return NULL ;
	}

	Malloc( lvl, LEVEL, 1 );
	__n_assert( lvl, return NULL );

	char *str = NULL ;
	Malloc( str, char, 1024 );
	__n_assert( str, return NULL );

	fgets( str, 1024, in );
	sscanf( str, "%d %d %d %d %d %d %d %lf %lf %lf %lf %lf %lf", &lvl -> w, &lvl -> h, &lvl -> m1, &lvl -> m2 , &lvl -> m3 , &lvl -> tilew, &lvl -> tileh , &lvl -> friction[ 0 ] , &lvl -> friction[ 1 ] , &lvl -> friction[ 2 ] , &lvl -> gravity[ 0 ] , &lvl -> gravity[ 1 ] , &lvl -> gravity[ 2 ] );
	fgets( str, 1024, in );
	sscanf( str, "%lf %lf" , &lvl -> startx , &lvl -> starty );

	for( int it = 0 ; it < 17 ; it ++ )
	{
		char tmpstr[ 1024 ] = "" ;
		fscanf( in, "%s", tmpstr );
		n_log( LOG_DEBUG, "loading tile id %d bmp %s", it , tmpstr );
		__n_assert( ( lvl -> tiles[ it ] = al_load_bitmap( tmpstr ) ) , n_log( LOG_ERR , "load bitmap %s returned null" , tmpstr ); );
	}

	n_log( LOG_DEBUG, "Level parameters: %d %d %d %d %d %d %d, start: %lf %lf", lvl -> w, lvl -> h, lvl -> m1 , lvl -> m2 , lvl -> m3 , lvl -> tilew, lvl -> tileh, lvl -> startx , lvl -> starty  );

	init_particle_system( &lvl -> particle_system_bullets, 5000, 0, 0, 0, 100 );
	init_particle_system( &lvl -> particle_system_effects, 5000, 0, 0, 0, 100 );

	lvl -> cells = (int **)calloc( lvl -> w, sizeof( void * ) );
	for( int x = 0 ; x < lvl -> w ; x ++ )
	{
		lvl -> cells[ x ] = (int *)calloc( lvl -> h, sizeof( void * ) );
	}
	for( int x = 0 ; x < lvl -> w ; x ++ )
		for( int y = 0 ;  y < lvl -> h ; y ++ )
			lvl -> cells[ x ][ y ] = 0 ;

	char buffer[ 16384 ] = "" ;
	int it_w = 0 , it_h = 0 , max_w = 0  ;
	while( fgets( buffer , 16384 , in ) && it_h < lvl -> h )
	{
		int len = strlen( buffer );
		if( len >  lvl -> w ) len = lvl -> w ;
		if( len > max_w ) max_w = len ;
		for( it_w = 0 ; it_w < len ; it_w ++ )
		{
			char test_str[ 4 ] = "" ;
			test_str[ 0 ] = buffer[ it_w ] ;
			test_str[ 1 ] = '\0' ;
			test_str[ 2 ] = '\0' ;
			test_str[ 3 ] = '\0' ;
			if( isdigit( buffer[ it_w ] ) )
			{
				lvl -> cells[ it_w ][ it_h ] = atoi( test_str );
			}
			else
			{
				switch( buffer[ it_w ] )
				{
					case 'S':
						lvl -> cells[ it_w ][ it_h ] = SWORD_CELL ;
						break ;
					case 'P':
						lvl -> cells[ it_w ][ it_h ] = SHIELD_CELL ;
						break ;
					case 'W':
						lvl -> cells[ it_w ][ it_h ] = WAND_CELL ;
						break ;
					case 'B':
						lvl -> cells[ it_w ][ it_h ] = BOW_CELL ;
						break ;
					case 'U':
						lvl -> cells[ it_w ][ it_h ] = KEY_FRAG1 ;
						break ;
					case 'I':
						lvl -> cells[ it_w ][ it_h ] = KEY_FRAG2 ;
						break ;
					case 'O':
						lvl -> cells[ it_w ][ it_h ] = KEY_FRAG3 ;
						break ;
				}
			}
		}
		it_h++;
	}
	n_log( LOG_DEBUG , "Got %d / %d max_size for level" , max_w , it_h );

	lvl -> monster_list = new_generic_list( -1 );

	// setting player pos if needed
	PHYSICS startpoint ;
	VECTOR3D_SET( startpoint . position, lvl -> startx * lvl -> tilew ,  lvl -> starty * lvl -> tileh , 0.0 );

	//placing passive monsters
	for( int it = 0 ; it < lvl -> m1 ; it ++ )
	{
		PHYSICS mpos ;
		int x = 0 , y = 0 ;
		mpos . can_jump = 1 ;
		do
		{
			do
			{
				x = rand()%max_w;
				y = rand()%it_h;
				VECTOR3D_SET( mpos . position, x * lvl -> tilew + lvl -> tilew / 2  , y * lvl -> tileh + lvl -> tileh / 2 , 0.0 );
			}
			while( distance( &mpos. position , &startpoint . position ) < 500 );
		}
		while( lvl -> cells[ x ][ y ] < PASSING_CELL_1 || lvl -> cells[ x ][ y ] > PASSING_CELL_2 );
		VECTOR3D_SET( mpos . speed, 0.0, 0.0, 0.0 );
		VECTOR3D_SET( mpos . acceleration, 0.0, 0.0, 0.0 );
		VECTOR3D_SET( mpos . gravity , lvl -> gravity[ 0 ] , lvl -> gravity[ 1 ] , lvl -> gravity[ 2 ] );
		VECTOR3D_SET( mpos . orientation, 0.0, 0.0, 0.0 );
		monster = new_monster( 100, PASSIVE_MONSTER , mpos );
		monster -> anim . id = 0 ;
		monster -> anim . lib = animations ;
		monster -> anim . frame = rand()%animations -> gfxs[ 0 ] -> nb_frames ;

		if( monster )
			list_push( lvl -> monster_list, monster, NULL );
	}
	//placing aggressive monsters
	for( int it = 0 ; it < lvl -> m2 ; it ++ )
	{
		PHYSICS mpos ;
		int x = 0 , y = 0 ;
		mpos . can_jump = 1 ;
		do
		{
			do
			{
				x = rand()%max_w;
				y = rand()%it_h;
				VECTOR3D_SET( mpos . position, x * lvl -> tilew + lvl -> tilew / 2  , y * lvl -> tileh + lvl -> tileh / 2 , 0.0 );
			}
			while( distance( &mpos. position , &startpoint . position ) < 1000 );
		}
		while( lvl -> cells[ x ][ y ] < PASSING_CELL_1 || lvl -> cells[ x ][ y ] > PASSING_CELL_2 );
		VECTOR3D_SET( mpos . speed, 0.0, 0.0, 0.0 );
		VECTOR3D_SET( mpos . acceleration, 0.0, 0.0, 0.0 );
		VECTOR3D_SET( mpos . gravity , lvl -> gravity[ 0 ] , lvl -> gravity[ 1 ] , lvl -> gravity[ 2 ] );
		VECTOR3D_SET( mpos . orientation, 0.0, 0.0, 0.0 );
		monster = new_monster( 150, AGRESSIVE_MONSTER , mpos );
		monster -> anim . id = 1 ;
		monster -> anim . lib = animations ;
		monster -> anim . frame = rand()%animations -> gfxs[ 1 ] -> nb_frames ;
		if( monster )
			list_push( lvl -> monster_list, monster, NULL );
	}
	//placing big boss monsters
	for( int it = 0 ; it < lvl -> m3 ; it ++ )
	{
		PHYSICS mpos ;
		int x = 0 , y = 0 ;
		mpos . can_jump = 1 ;
		do
		{
			do
			{
				x = rand()%max_w;
				y = rand()%it_h;
				VECTOR3D_SET( mpos . position, x * lvl -> tilew + lvl -> tilew / 2  , y * lvl -> tileh + lvl -> tileh / 2 , 0.0 );
			}
			while( distance( &mpos. position , &startpoint . position ) < 1000 );
		}
		while( lvl -> cells[ x ][ y ] < PASSING_CELL_1 || lvl -> cells[ x ][ y ] > PASSING_CELL_2 );


		VECTOR3D_SET( mpos . speed, 0.0, 0.0, 0.0 );
		VECTOR3D_SET( mpos . acceleration, 0.0, 0.0, 0.0 );
		VECTOR3D_SET( mpos . gravity , lvl -> gravity[ 0 ] , lvl -> gravity[ 1 ] , lvl -> gravity[ 2 ] );
		VECTOR3D_SET( mpos . orientation, 0.0, 0.0, 0.0 );
		monster = new_monster( 1000, BIGBOSS_MONSTER , mpos );
		monster -> anim . id = 2 ;
		monster -> anim . lib = animations ;
		monster -> anim . frame = rand()%animations -> gfxs[ 2 ] -> nb_frames ;
		if( monster )
			list_push( lvl -> monster_list, monster, NULL );
	}

	// last dungeon, place boss monster
	if( !strcmp( _nstr( (*player) . attr . level_string ) , "DATA/Levels/dungeon_4.txt" ) )
	{
		PHYSICS mpos ;
		mpos . can_jump = 1 ;
		VECTOR3D_SET( mpos . position, 5 * lvl -> tilew + lvl -> tilew / 2  , 190 * lvl -> tileh + lvl -> tileh / 2 , 0.0 );
		VECTOR3D_SET( mpos . speed, 0.0, 0.0, 0.0 );
		VECTOR3D_SET( mpos . acceleration, 0.0, 0.0, 0.0 );
		VECTOR3D_SET( mpos . gravity , lvl -> gravity[ 0 ] , lvl -> gravity[ 1 ] , lvl -> gravity[ 2 ] );
		VECTOR3D_SET( mpos . orientation, 0.0, 0.0, 0.0 );
		monster = new_monster( 3000 , FINAL_MONSTER , mpos );
		monster -> anim . id = 11 ;
		monster -> anim . lib = animations ;
		monster -> anim . frame = rand()%animations -> gfxs[ 11 ] -> nb_frames ;
		if( monster )
			list_push( lvl -> monster_list, monster, NULL );
	}

	lvl -> native_w = w ;
	lvl -> native_h = h ;

	n_log( LOG_INFO, "Level %s loaded", _nstr( (*player) . attr . level_string ) );

	return lvl ;
}



int draw_level( LEVEL *lvl, PLAYER *player, int w, int h )
{
	int startx =  ( player -> physics . position[ 0 ] - w / 2 ) / lvl -> tilew ;
	int starty =  ( player -> physics . position[ 1 ] - h / 2 ) / lvl -> tileh ;
	int endx = 1 + w / lvl -> tilew ;
	int endy = 1 + h / lvl -> tileh ;

	int mx = 0, my = 0 ;

	mx = ( player -> physics . position[ 0 ] - w / 2 ) - lvl -> tilew * startx ;
	my = ( player -> physics . position[ 1 ] - h / 2 ) - lvl -> tileh * starty ;


	double xmin = 0.0, ymin = 0.0, xmax = 0.0, ymax = 0.0 ;
	for( int x = 0 ; x <= endx ; x ++ )
	{
		for( int y = 0 ; y <= endy ; y ++ )
		{
			int x1 = x * lvl -> tilew - mx ;
			int y1 = y * lvl -> tileh - my ;

			int x2 = x1 + lvl -> tilew ;
			int y2 = y1 + lvl -> tileh ;

			if( x1 < xmin )
				xmin = x1 ;
			if( x2 < xmin )
				xmin = x2 ;
			if( y1 < ymin )
				ymin = y1 ;
			if( y2 < ymin )
				ymin = y2 ;
			if( x1 > xmax )
				xmax = x1 ;
			if( x2 > xmax )
				xmax = x2 ;
			if( y1 > ymax )
				ymax = y1 ;
			if( y2 > ymax )
				ymax = y2 ;


			int px = x + startx ;
			int py = y + starty ;

			if( px >= 0 && py >= 0 && px < lvl -> w && py < lvl -> h )
			{
				if( lvl -> cells[ px ][ py ] >= 0 )
				{
					ALLEGRO_BITMAP *bmp = lvl -> tiles[ lvl -> cells[ px ][ py ] ];
					if( bmp )
					{
						int w = al_get_bitmap_width( bmp );
						int h = al_get_bitmap_height( bmp );

						al_draw_scaled_bitmap( bmp ,
								0 , 0 , w , h ,
								x1 , y1 , lvl -> tilew , lvl -> tileh , 0 );

						//al_draw_bitmap( bmp, x1 + (lvl -> tilew / 2) - w / 2, y1 - h + lvl -> tileh, 0 );
					}
					else
					{
						al_draw_filled_rectangle( x1, y1, x2, y2, al_map_rgb( 255, 255, 255 ) );
					}
				}
				else
				{
					al_draw_filled_rectangle( x1, y1, x2, y2, al_map_rgb( 255, 255, 255 ) );
				}
			}
		}
	}
	//al_draw_filled_circle( WIDTH / 2, HEIGHT/2 , player . physics . sz ,  al_map_rgb( 255 - ( 255 * player . attr . life ) / 100 , 255 * player . attr . life / 100 , 0 ) );
	draw_anim( &player -> anim , WIDTH / 2 , HEIGHT/2 + player -> physics . sz );

	LIST_NODE *node = NULL ;
	node = lvl -> monster_list -> start ;
	int it = 0 ;
	while( node )
	{
		MONSTER *monster = node -> ptr ;
		if( monster )
		{
			double x = w / 2 + monster -> physics . position[ 0 ] - player -> physics . position[ 0 ] - monster -> physics .sz ;
			double y = h / 2 + monster -> physics . position[ 1 ] - player -> physics . position[ 1 ] ;

			if( monster -> attr . type == FINAL_MONSTER )
			{
				if( monster -> physics . speed[ 0 ] > 0 )
				{
					monster -> anim . id = 11 ;
				}
				if( monster -> physics . speed[ 0 ] < 0 )
				{
					monster -> anim . id = 12 ;
				}
			}

			draw_anim( &monster -> anim , x + monster -> physics .sz , y + monster -> physics .sz );
			if( monster -> attr . type == PASSIVE_MONSTER )
			{
				if( monster -> attr . action == 1 )
				{
					PHYSICS tmp_part ;
					tmp_part . sz = 1  ;
					tmp_part . type = 2 ;
					VECTOR3D_SET( tmp_part . speed, 100 - rand()%200, 100 - rand()%200, 0.0  );
					VECTOR3D_SET( tmp_part . position, monster -> physics . position[ 0 ], monster ->  physics . position[ 1 ] - monster -> physics . sz , 0.0  );
					VECTOR3D_SET( tmp_part . acceleration, 0.0, 0.0, 0.0 );
					VECTOR3D_SET( tmp_part . orientation, 0.0, 0.0, 0.0 );
					int color = rand()%255;
					add_particle( lvl -> particle_system_effects, -1, PIXEL_PART, rand()%500, 1+rand()%2, al_map_rgba( color , color , 0, rand()%100 ), tmp_part );
				}
			}
			else if( monster -> attr . type == AGRESSIVE_MONSTER )
			{
				if( monster -> attr . action == 1 )
				{
					PHYSICS tmp_part ;
					tmp_part . sz = 1  ;
					tmp_part . type = 2 ;
					VECTOR3D_SET( tmp_part . speed, 100 - rand()%200, 100 - rand()%200, 0.0  );
					VECTOR3D_SET( tmp_part . position, monster -> physics . position[ 0 ], monster ->  physics . position[ 1 ] - monster -> physics . sz, 0.0  );
					VECTOR3D_SET( tmp_part . acceleration, 0.0, 0.0, 0.0 );
					VECTOR3D_SET( tmp_part . orientation, 0.0, 0.0, 0.0 );
					add_particle( lvl -> particle_system_effects, -1, PIXEL_PART, rand()%500, 1+rand()%3, al_map_rgba( rand()%255, 0 , 0, rand()%100 ), tmp_part );
				}
			}
			else if( monster -> attr . type == BIGBOSS_MONSTER )
			{
				if( monster -> attr . action == 1 )
				{
					PHYSICS tmp_part ;
					tmp_part . sz = 1  ;
					tmp_part . type = 2 ;
					VECTOR3D_SET( tmp_part . speed, 100 - rand()%200, 100 - rand()%200, 0.0  );
					VECTOR3D_SET( tmp_part . position, monster -> physics . position[ 0 ], monster ->  physics . position[ 1 ], 0.0  );
					VECTOR3D_SET( tmp_part . acceleration, 0.0, 0.0, 0.0 );
					VECTOR3D_SET( tmp_part . orientation, 0.0, 0.0, 0.0 );
					add_particle( lvl -> particle_system_effects, -1, PIXEL_PART, rand()%500, 1+rand()%4, al_map_rgba( 0, 0,rand()%255 , rand()%100 ), tmp_part );
				}
			}
			else if( monster -> attr . type == FINAL_MONSTER )
			{
				if( monster -> attr . action == 1 )
				{
					PHYSICS tmp_part ;
					tmp_part . sz = 1  ;
					tmp_part . type = 2 ;
					VECTOR3D_SET( tmp_part . speed, 100 - rand()%200, 100 - rand()%200, 0.0  );
					VECTOR3D_SET( tmp_part . position, monster -> physics . position[ 0 ], monster ->  physics . position[ 1 ], 0.0  );
					VECTOR3D_SET( tmp_part . acceleration, 0.0, 0.0, 0.0 );
					VECTOR3D_SET( tmp_part . orientation, 0.0, 0.0, 0.0 );
					add_particle( lvl -> particle_system_effects, -1, PIXEL_PART, rand()%500, 1+rand()%4, al_map_rgba( 0, 0 , 0, rand()%100 ), tmp_part );
				}
			}
		}
		node = node -> next ;
		it ++ ;
	}

	draw_particle( lvl -> particle_system_effects, player -> physics . position[ 0 ] - w/2, player -> physics . position[ 1 ] - h/2, w, h, 50 );
	draw_particle( lvl -> particle_system_bullets, player -> physics . position[ 0 ] - w/2, player -> physics . position[ 1 ] - h/2, w, h, 50 );

	return TRUE ;
}
