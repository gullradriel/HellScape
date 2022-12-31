/**\file n_crypto.c
 *  vigenere encode decode function
 *\author Castagnier Mickael
 *\version 1.0
 *\date 10/11/2022
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#define _GNU_SOURCE_WAS_NOT_DEFINED
#endif

#include "nilorea/n_common.h"
#include "nilorea/n_base64.h"
#include "nilorea/n_crypto.h"
#include "nilorea/n_thread_pool.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#ifndef __windows__
#include <sys/sysinfo.h>
#endif
#ifdef _GNU_SOURCE_WAS_NOT_DEFINED
#undef _GNU_SOURCE
#endif

#ifdef __windows__
#include <windows.h>
#endif



/*! static internal root key, just to obfuscate text so we do not care if is is public */
static char *__internal_root_key = "ENOFGLUCUNZJADRDMCZZSACRBKXAGSLTTFCICPALKHMWWGLVUIFYSLQJIHHXSHOZGRABFNBHGVTOMEBTVIPXZJHHEKIYWNVTWQKERROCTXFGMMLYUJSJFWLFCHQQMMUM";



/*!\fn N_STR *n_vigenere_get_rootkey( size_t rootkey_size )
 * \brief get a rootkey randomly generated
 * \param rootkey_size size of the needed rootkey. Must be a multiple of 2
 * \return a N_STR *rootkey string
 */
N_STR *n_vigenere_get_rootkey( size_t rootkey_size )
{
	if( ( rootkey_size % 2 ) != 0 )
	{
		n_log( LOG_ERR , "key size %d is not a multiple of 2" , rootkey_size );
		return NULL ;
	}
	N_STR *output = new_nstr( rootkey_size + 1 );
	srand( time( NULL ) );
	for( size_t it = 0 ; it < rootkey_size ; it ++ )
	{
		output -> data[ it ] = 'A' + rand()%26 ;
	}
	output -> written = rootkey_size ;
	return output ;
}



/*!\fn N_STR *n_get_current_dir_hd_serial()
 * \brief get the serial of the current running dir hosting disk
 * \return a N_STR *serial which contain actual running path disk id
 */
N_STR *n_get_current_dir_hd_serial()
{
	N_STR *output = NULL ;

#ifdef __windows__
	DWORD dwVolSerial;
	BOOL bIsRetrieved;
	bIsRetrieved = GetVolumeInformation(NULL,NULL,0,&dwVolSerial,NULL,NULL,NULL,0);

	if (bIsRetrieved)
	{
		nstrprintf( output , "%X" , (unsigned int)dwVolSerial );
	} else {
		n_log( LOG_ERR , "Could not retrieve current directory disk serial id" );
		return NULL ;
	}

	return output ;
#else
	char *curdir = getcwd( NULL , 0 );

	N_STR *cmd = NULL ;
	nstrprintf( cmd , "df -T \"%s\" | awk '/^\\/dev/ {print $1}'" , curdir );
	Free( curdir );

	N_STR *cur_disk = NULL ;
	int ret = -1 ;
	n_popen( _nstr( cmd ) , 1024 , (void **)&cur_disk , &ret );
	free_nstr( &cmd );

	nstrprintf( cmd , "udevadm info --query=all --name=%s | egrep '(ID_SERIAL=|ID_FS_UUID=)' | awk -F'=' '{print $2}'" , _nstr( cur_disk ) );
	n_popen( _nstr( cmd ) , 1024 , (void **)&output , &ret );
	free_nstr( &cmd );

	N_STR *hd_serial = NULL ;
	nstrprintf( hd_serial , "%s=>%s" , _nstr( output ) , _nstr( cur_disk ) );

	free_nstr( &output );
	free_nstr( &cur_disk );

	for( unsigned int it = 0 ; it < hd_serial -> written ; it ++ )
	{
		if( isspace( hd_serial -> data[ it ] ) )
		{
			hd_serial -> data[ it ] = '-' ;
		}
	}

	return hd_serial ;

#endif
}



/*!\fn N_STR *n_get_cpu_id()
 * \brief get the CPU id
 * \return a N_STR *cpuid 
 */
N_STR *n_get_cpu_id()
{
	int a[ 5 ] = { 0 , 0 , 0 , 0 , 0 };

	__asm__("mov $0x1 , %eax\n\t");
	__asm__("cpuid\n\t");
	__asm__("mov %%eax, %0\n\t":"=r" (a[0])); //gives model and family
	__asm__("mov %%ebx, %0\n\t":"=r" (a[1])); //gives additional feature info
	__asm__("mov %%ecx, %0\n\t":"=r" (a[2])); //feature flags
	__asm__("mov %%edx, %0\n\t":"=r" (a[3])); //feature flags

	int binaryNum[ 30 ];
	memset( &binaryNum , 0 , sizeof( binaryNum ) );

	int n = a[ 3 ];
	int i = 0;
	while (n > 0) {

		// storing remainder in binary array
		binaryNum[i] = n % 2;
		n = n / 2;
		i++;
	}
	n = a[ 2 ];
	i = 0;
	while (n > 0) {

		// storing remainder in binary array
		binaryNum[i] = n % 2;
		n = n / 2;
		i++;
	}
	n = a[ 1 ];
	i = 0;
	while (n > 0) {

		// storing remainder in binary array
		binaryNum[i] = n % 2;
		n = n / 2;
		i++;
	}
	n = a[ 0 ];
	i = 0;
	while (n > 0) {

		// storing remainder in binary array
		binaryNum[i] = n % 2;
		n = n / 2;
		i++;
	}

	/*  printf("-------Signature(EAX register):-------");
		printf("\nStepping ID:%d%d%d%d",binaryNum[3],binaryNum[2],binaryNum[1],binaryNum[0]);
		printf("\nModel:%d%d%d%d",binaryNum[7],binaryNum[6],binaryNum[5],binaryNum[4]);
		printf("\nFamily ID:%d%d%d%d",binaryNum[11],binaryNum[10],binaryNum[9],binaryNum[8]);
		printf("\nProcessor Type:%d%d",binaryNum[13],binaryNum[12]);
		printf("\nExtended Model ID:%d%d%d%d",binaryNum[19],binaryNum[18],binaryNum[17],binaryNum[16]);
		printf("\nExtended Family ID:%d%d%d%d%d%d%d%d",binaryNum[27],binaryNum[26],binaryNum[25],binaryNum[24],binaryNum[23],binaryNum[22],binaryNum[21],binaryNum[20]);
		printf("\n"); */

	N_STR *cpu_id = NULL ;
	nstrprintf( cpu_id , "%d%d%d%d-%d%d%d%d-%d%d%d%d-%d%d-%d%d%d%d-%d%d%d%d%d%d%d%d" , binaryNum[3],binaryNum[2],binaryNum[1],binaryNum[0] , binaryNum[7],binaryNum[6],binaryNum[5],binaryNum[4] , binaryNum[11],binaryNum[10],binaryNum[9],binaryNum[8] , binaryNum[13],binaryNum[12] , binaryNum[19],binaryNum[18],binaryNum[17],binaryNum[16] , binaryNum[27],binaryNum[26],binaryNum[25],binaryNum[24],binaryNum[23],binaryNum[22],binaryNum[21],binaryNum[20] );

	return cpu_id ;
}

/*! structure of a n_vigenere_cypher_thread param */
typedef struct CYPHER_PARAM
{
	/*! input data */
	N_STR *input ;
	/*! destination */
	N_STR *dest ;
	/*! encoding/decoding key */
	N_STR *key ;
	/*! 1 encoding, 0 decoding */
	bool encipher ;
	/*! input starting point */
	size_t start_pos ;
	/*! output starting point */
	size_t end_pos ;
} CYPHER_PARAM ;


/*!\fn void *n_vigenere_cypher_thread( void *ptr )
 * \brief encode or decode threaded func using params from ptr
 * \param ptr a CYPHER_PARAM *param struc 
 */
void *n_vigenere_cypher_thread( void *ptr )
{
	__n_assert( ptr , return NULL );
	CYPHER_PARAM *params = (CYPHER_PARAM *)ptr ;
	__n_assert( params , return NULL );

	N_STR *input = params -> input ;
	N_STR *dest = params -> dest ;
	N_STR *key = params -> key ;

	__n_assert( input , return NULL );
	__n_assert( input -> data , return NULL );
	__n_assert( key , return NULL );
	__n_assert( key -> data , return NULL );

	bool encipher = params -> encipher ;

	size_t start_pos = params -> start_pos ; 
	size_t end_pos = params -> end_pos ; 

	bool cIsUpper = 0 ;
	uint8_t keyIndexMod = 0;
	if( encipher )
	{
		for( size_t i = start_pos ; i < end_pos ; i++ ) 
		{
			if( ( cIsUpper = n_isupper( input -> data[ i ] ) ) || ( n_islower( input -> data[ i ] ) ) )
			{
				char offset = cIsUpper ? 'A' : 'a';
				keyIndexMod= i % key -> written ;
				int k = ( cIsUpper ? n_toupper( key -> data[ keyIndexMod ] ) : n_tolower( key -> data[ keyIndexMod ] ) ) - offset ;
				dest -> data[ i ] = (char)( ( n_mathmod( ( ( input -> data[ i ] + k ) - offset ) , 26 ) ) + offset );
			}
		}
	}
	else
	{
		for( uint32_t i = start_pos ; i < end_pos ; i++ ) 
		{
			if( ( cIsUpper = n_isupper( input -> data[ i ] ) ) || ( n_islower( input -> data[ i ] ) ) )
			{
				char offset = cIsUpper ? 'A' : 'a';
				keyIndexMod= i % key -> written ;
				int k = ( cIsUpper ? n_toupper( key -> data[ keyIndexMod ] ) : n_tolower( key -> data[ keyIndexMod ] ) ) - offset ;
				dest -> data[ i ] = (char)( ( n_mathmod( ( ( input -> data[ i ] - k ) - offset ) , 26 ) ) + offset );
			}
		}
	}
	Free( params );
	return NULL ;
}




/*!\fn N_STR *n_vigenere_cypher( N_STR *input , N_STR *key , bool encipher , bool in_place )
 * \brief encode or decode input using vigenere cypher
 * \param input source to encode/decode
 * \param key key to use for the encoding/decoding
 * \param encipher bool, set to 1 => encode, 0 => decode
 * \param in_place bool, set to 1 => use input as output string , 0 return a new encoded copy
 * \return a N_STR *encoded or *decoded string 
 */
N_STR *n_vigenere_cypher( N_STR *input , N_STR *key , bool encipher , bool in_place )
{
	__n_assert( input , return NULL );
	__n_assert( input -> data , return NULL );
	__n_assert( key , return NULL );
	__n_assert( key -> data , return NULL );

	if( input -> written == 0 )
	{
		n_log( LOG_ERR , "empty input, written=0" );
		return NULL ;
	}

	if( ( key -> written % 2 ) != 0 )
	{
		n_log( LOG_ERR , "odd number of elements in root key %s, %d" , _nstr( key ) , key -> written );
		return NULL ;
	}

	for( size_t i = 0 ; i < key -> written ; ++i )
	{
		if( !isalpha( key -> data[ i ] ) )
		{
			n_log( LOG_ERR , "key contain bad char '%c'" ,  key -> data[ i ] );
			return NULL ; // Error
		}
	}

	N_STR *dest = NULL ;
	if( in_place)
	{
		dest = input ;
	}
	else
	{
		dest = nstrdup( input );
	}

#ifdef __windows__
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	int nb_procs = sysinfo.dwNumberOfProcessors;
#else
	int nb_procs = get_nprocs();
#endif


	if( nb_procs < 2 )
		nb_procs = 2 ;

	THREAD_POOL *thread_pool = new_thread_pool( nb_procs , 0 );

	size_t bytesteps =  input -> written / nb_procs ;
	if( bytesteps == 0 ) bytesteps = input -> written ;

	size_t end_pos = 0 ; 
	for( size_t it = 0 ; it < input -> written ; it += bytesteps )
	{
		end_pos = it + bytesteps ; 

		if( end_pos >= input -> written )
			end_pos = input -> written ;

		CYPHER_PARAM *params = NULL ;
		Malloc( params , CYPHER_PARAM , 1 );
		params -> input = input ;
		params -> dest = dest ;
		params -> key = key ;
		params -> encipher = encipher ;
		params -> start_pos = it ;
		params -> end_pos = end_pos ;

		if( add_threaded_process( thread_pool, &n_vigenere_cypher_thread, (void *)(intptr_t)params, DIRECT_PROC) == FALSE )
		{
			n_log( LOG_ERR, "Error adding n_vigenere_cypher_thread to thread pool" );
		}
		refresh_thread_pool( thread_pool );
		usleep( 1 );

	}
	if( end_pos <  input -> written )
	{
		CYPHER_PARAM *params = NULL ;
		Malloc( params , CYPHER_PARAM , 1 );
		params -> input = input ;
		params -> dest = dest ;
		params -> key = key ;
		params -> encipher = encipher ;
		params -> start_pos = end_pos ;
		params -> end_pos = input -> written ;

		if( add_threaded_process( thread_pool, &n_vigenere_cypher_thread, (void *)(intptr_t)params, DIRECT_PROC) == FALSE )
		{
			n_log( LOG_ERR, "Error adding n_vigenere_cypher_thread to thread pool" );
		}
		refresh_thread_pool( thread_pool );
		usleep( 1 );
	}
	wait_for_threaded_pool( thread_pool, 100000 );
	destroy_threaded_pool( &thread_pool, 100000 );

	/*
	   uint32_t nonAlphaCharCount = 0;
	   for( uint32_t i = 0 ; i < input -> written ; i++ ) 
	   {
	   char ch = input -> data[ i ];
	   if( isalpha( input -> data[ i ] ) )
	   {
	   bool cIsUpper = isupper( input -> data[ i ] );
	   char offset = cIsUpper ? 'A' : 'a';
	   int keyIndex = ( i - nonAlphaCharCount ) % key -> written ;
	   int k = ( cIsUpper ? toupper( key -> data[ keyIndex % key -> written ] ) : tolower( key -> data[ keyIndex % key -> written ] ) ) - offset ;
	   k = encipher ? k : -k;
	   ch = (char)( ( n_mathmod( ( ( input -> data[ i ] + k ) - offset ) , 26 ) ) + offset );
	   }
	   else
	   {
	   ++nonAlphaCharCount;
	   }
	   dest -> data[ i ] = ch;
	   }
	   */

	return dest;
}


/*!\fn N_STR *n_vigenere_encode( N_STR *string , N_STR *key )
 * \brief encode input using vigenere cypher and key
 * \param string source to encode
 * \param key key to use for the encoding/decoding
 * \return a N_STR *encoded 
 */
N_STR *n_vigenere_encode( N_STR *string , N_STR *key )
{
	return n_vigenere_cypher( string , key , 1 , 0 );
}



/*!\fn N_STR *n_vigenere_decode( N_STR *string , N_STR *key )
 * \brief decode input using vigenere cypher and key
 * \param string source to decode
 * \param key key to use for the encoding/decoding
 * \return a N_STR *decoded
 */
N_STR *n_vigenere_decode( N_STR *string , N_STR *key )
{
	return n_vigenere_cypher( string , key , 0 , 0 );
}



/*!\fn N_STR *n_vigenere_encode_in_place( N_STR *string , N_STR *key )
 * \brief encode string in place using vigenere cypher and key
 * \param string source to encode
 * \param key key to use for the encoding/decoding
 * \return A pointer to string or an error 
 */
N_STR *n_vigenere_encode_in_place( N_STR *string , N_STR *key )
{
	return n_vigenere_cypher( string , key , 1 , 1 );
}



/*!\fn N_STR *n_vigenere_decode_in_place( N_STR *string , N_STR *key )
 * \brief decode input using vigenere cypher and key
 * \param string source to decode
 * \param key key to use for the encoding/decoding
 * \return a N_STR *decoded
 */
N_STR *n_vigenere_decode_in_place( N_STR *string , N_STR *key )
{
	return n_vigenere_cypher( string , key , 0 , 1 );
}



/*!\fn N_STR *n_vigenere_quick_encode( N_STR *decoded_data );
 * \brief quick encode data
 * \param decoded_data the data to encode
 * \return a N_STR *encoded_data
 */
N_STR *n_vigenere_quick_encode( N_STR *decoded_data )
{
	N_STR *__internal_root_key_nstr = char_to_nstr( __internal_root_key );

	N_STR *base64_data = n_base64_encode( decoded_data );
	N_STR *encoded_data = n_vigenere_encode_in_place( base64_data , __internal_root_key_nstr );
	free_nstr( &__internal_root_key_nstr );

	return encoded_data ;
}



/*!\fn N_STR *n_vigenere_quick_decode( N_STR *encoded_data );
 * \brief quick decode data
 * \param encoded_data the data to decode
 * \return a N_STR *decoded_data
 */
N_STR *n_vigenere_quick_decode( N_STR *encoded_data )
{
	N_STR *__internal_root_key_nstr = char_to_nstr( __internal_root_key );

	N_STR *decoded_data = n_vigenere_decode( encoded_data , __internal_root_key_nstr );
	N_STR *base64_data = n_base64_decode( decoded_data );

	free_nstr( &__internal_root_key_nstr );
	free_nstr( &decoded_data );

	return base64_data ;
}



/*!\fn N_STR *n_vigenere_get_question( size_t question_size )
 * \brief get a question generated from the current machine hardware (disk&cpu)
 * \param question_size size of the needed question. Must be a multiple of 2
 * \return a N_STR *question string
 */
N_STR *n_vigenere_get_question( size_t question_size )
{
	if( ( question_size % 2 ) != 0 )
	{
		n_log( LOG_ERR , "odd number of elements for question size, %d" , question_size );
		return NULL ;
	}

	N_STR *output = NULL ;
	output = n_get_current_dir_hd_serial();

	N_STR *cpu_id = n_get_cpu_id();
	nstrprintf_cat( output , "@%s" , _nstr( cpu_id ) );
	free_nstr( &cpu_id );

	if( output )
	{
		N_STR *tmpoutput = n_base64_encode( output );
		free_nstr( &output );
		output = new_nstr( question_size + 1 );
		for( size_t it = 0 ; it < question_size ; it ++ )
		{
			output -> data[ it ] = tmpoutput -> data[ it % tmpoutput -> written ] ;

			while( output -> data[ it ] < 'A' ) output -> data[ it ]  += 26 ;
			while( output -> data[ it ] > 'Z' ) output -> data[ it ] -= 26 ;
		}
		output -> written = question_size ;
		free_nstr( &tmpoutput );
	}
	else
	{
		// generate random
		output = new_nstr( question_size + 1 );
		for( size_t it = 0 ; it < question_size ; it ++ )
		{
			output -> data[ it ] = rand()%255 ;
			while( output -> data[ it ] < 'A' ) output -> data[ it ]  += 26 ;
			while( output -> data[ it ] > 'Z' ) output -> data[ it ] -= 26 ;
		}
	}

	N_STR *encoded_data = n_vigenere_quick_encode( output );
	free_nstr( &output );

	size_t it = 0 ;
	while( it < encoded_data -> written )
	{
		if( encoded_data -> data[ it ] == '=' )
		{
			encoded_data -> data[ it ] = '\0' ;
		}
		it ++ ;
	}
	return encoded_data ;
}



/*!\fn N_STR *n_vigenere_get_answer( N_STR *root_key , N_STR *question )
 * \brief get an answer from a root key and a question
 * \param root_key the key used to do the inital encoding
 * \param question question generated from hardware
 * \return a N_STR *answer_key string 
 */
N_STR *n_vigenere_get_answer( N_STR *root_key , N_STR *question )
{
	__n_assert( root_key , return NULL );
	__n_assert( question , return NULL );

	N_STR *decoded_question = n_vigenere_quick_decode( question );

	if( root_key -> written < decoded_question -> written )
	{
		n_log( LOG_ERR , "root key of size %d is lower than question key of size %d" , root_key -> written , decoded_question -> written );
		free_nstr( &decoded_question );
		return NULL ;
	}
	if( (root_key -> written % decoded_question -> written) != 0 )
	{
		n_log( LOG_ERR , "question key of size %d is not a multiple of root key of size %d" , decoded_question -> written , root_key -> written );
		free_nstr( &decoded_question );
		return NULL ;
	}

	N_STR *answer = new_nstr( root_key -> length + 1 );
	for( uint32_t it = 0 ; it < root_key -> written ; it ++ )
	{
		int32_t val = ( root_key -> data[ it % root_key -> written ] - 'A' ) + ( decoded_question -> data[ it % decoded_question -> written ] - 'A' );
		val = val % 26 ;
		val = val + 'A' ;
		answer -> data[ it ] = val ;
	}
	free_nstr( &decoded_question );
	answer -> written = root_key -> written ;
	return answer ;
}




/*!\fn int n_vigenere_encode_file( N_STR *in , N_STR *out , N_STR *key )
 * \brief directly vigenere encode a file using key
 * \param in input path and filename
 * \param out output path and filename
 * \param key key to use
 * \return TRUE or FALSE
 */
int n_vigenere_encode_file( N_STR *in , N_STR *out , N_STR *key )
{
	__n_assert( in , return FALSE );
	__n_assert( out , return FALSE );
	__n_assert( key , return FALSE );

	if( ( key -> written % 2 ) != 0 )
	{
		n_log( LOG_ERR , "odd number of elements in key %s, %d" , _nstr( key ) , key -> written );
		return FALSE ;
	}

	N_STR *file = file_to_nstr( _nstr( in ) );
	if( !file )
	{
		n_log( LOG_ERR , "error loading %s" , _nstr( in ) );
		return FALSE ;
	}

	N_STR *base64_data = n_base64_encode( file );
	free_nstr( &file );

	if( !base64_data )
	{
		n_log( LOG_ERR , "Could not base64 decode data from file %s" , _nstr( in ) );
		return FALSE ;
	}

	N_STR *encoded_data = n_vigenere_encode_in_place( base64_data , key );
	if( !encoded_data )
	{
		n_log( LOG_ERR , "Could not decode data from file %s" , _nstr( in ) );
		return FALSE ;
	}

	int ret = nstr_to_file( encoded_data , _nstr( out ) );
	free_nstr( &encoded_data );

	return ret ;
}



/*!\fn int n_vigenere_decode_file( N_STR *in , N_STR *out , N_STR *key )
 * \brief directly vigenere decode a file using key
 * \param in input path and filename
 * \param out output path and filename
 * \param key key to use
 * \return TRUE or FALSE
 */
int n_vigenere_decode_file( N_STR *in , N_STR *out , N_STR *key )
{
	__n_assert( in , return FALSE );
	__n_assert( out , return FALSE );
	__n_assert( key , return FALSE );

	if( ( key -> written % 2 ) != 0 )
	{
		n_log( LOG_ERR , "odd number of elements in key %s , %d" , _nstr( key ) , key -> written );
		return FALSE ;
	}

	N_STR *file = file_to_nstr( _nstr( in ) );
	if( !file )
	{
		n_log( LOG_ERR , "error loading %s" , _nstr( in ) );
		return FALSE ;
	}

	N_STR *decoded_data = n_vigenere_decode_in_place( file , key );
	if( !decoded_data )
	{
		n_log( LOG_ERR , "Could not decode data from file %s" , _nstr( in ) );
		return FALSE ;
	}

	N_STR *base64_data = n_base64_decode( decoded_data );
	free_nstr( &decoded_data );
	if( !base64_data )
	{
		n_log( LOG_ERR , "Could not base64 decode data from file %s" , _nstr( in ) );
		return FALSE ;
	}

	int ret = nstr_to_file( base64_data , _nstr( out ) );
	free_nstr( &base64_data );

	return ret ;
}



/*!\fn N_STR *n_vigenere_encode_qa( N_STR *input_data , N_STR *question , N_STR *answer )
 * \brief directly vigenere encode a file using key
 * \param input_data input path and filename
 * \param question question key to use
 * \param answer answer key to use
 * \return a N_STR *encoded string or NULL
 */
N_STR *n_vigenere_encode_qa( N_STR *input_data , N_STR *question , N_STR *answer )
{
	__n_assert( input_data , return NULL );
	__n_assert( question , return NULL );
	__n_assert( answer , return NULL );

	N_STR *b64_encoded = n_base64_encode( input_data );
	if( !b64_encoded )
	{
		n_log( LOG_ERR , "could not base64 decode: input_data %p" , input_data );
		return NULL ;
	}

	N_STR *encoded_with_answer_data = n_vigenere_encode_in_place( b64_encoded , answer );
	if( !encoded_with_answer_data )
	{
		n_log( LOG_ERR , "could not encode: input_data %p" , input_data );
		return NULL ;
	}

	N_STR *decoded_question = n_vigenere_quick_decode( question );
	N_STR *encoded_data = n_vigenere_decode_in_place( encoded_with_answer_data , decoded_question );
	free_nstr( &decoded_question );
	if( !encoded_data )
	{
		n_log( LOG_ERR , "could not decode: input_data %p" , input_data );
		return NULL ;
	}

	return encoded_data ;
}



/*!\fn N_STR *n_vigenere_decode_qa( N_STR *input_data , N_STR *question , N_STR *answer )
 * \brief directly vigenere decode a file using key
 * \param input_data input path and filename
 * \param question question key to use
 * \param answer answer key to use
 * \return a N_STR *decoded string or NULL
 */
N_STR *n_vigenere_decode_qa( N_STR *input_data , N_STR *question , N_STR *answer )
{
	__n_assert( input_data , return NULL );
	__n_assert( input_data -> data , return NULL );
	__n_assert( question , return NULL );
	__n_assert( question -> data , return NULL );
	__n_assert( answer , return NULL );
	__n_assert( answer -> data , return NULL );

	N_STR *decoded_question = n_vigenere_quick_decode( question );

	N_STR *encoded_data = n_vigenere_encode( input_data , decoded_question );
	free_nstr( &decoded_question );
	if( !encoded_data )
	{
		n_log( LOG_ERR , "could not encode input_data %p" , input_data );
		return NULL ;
	}
	n_log( LOG_DEBUG , "encoded_data: %d/%d" , encoded_data -> written , encoded_data -> length );

	N_STR *encoded_with_answer_data = n_vigenere_decode_in_place( encoded_data , answer );
	if( !encoded_with_answer_data )
	{
		n_log( LOG_ERR , "could not decode: input_data %p" , input_data );
		return NULL ;
	}
	n_log( LOG_DEBUG , "encoded_with_answer_data: %d/%d" , encoded_with_answer_data -> written , encoded_with_answer_data -> length );

	N_STR *b64_decoded = n_base64_decode( encoded_with_answer_data );
	free_nstr( &encoded_with_answer_data );
	if( !b64_decoded )
	{
		n_log( LOG_ERR , "could not base64 decode: input_data %p" , input_data );
		return NULL ;
	}
	n_log( LOG_DEBUG , "b64_decoded: %d/%d" , b64_decoded -> written , b64_decoded -> length );
	return b64_decoded ;
}



/*!\fn int n_vigenere_decode_file_qa( N_STR *in , N_STR *out , N_STR *question , N_STR *answer )
 * \brief directly vigenere decode a file using question and answer
 * \param in input path and filename
 * \param out output path and filename
 * \param question question key to use
 * \param answer answer key to use
 * \return TRUE or FALSE
 */
int n_vigenere_decode_file_qa( N_STR *in , N_STR *out , N_STR *question , N_STR *answer )
{
	__n_assert( in , return FALSE );
	__n_assert( out , return FALSE );
	__n_assert( question , return FALSE );
	__n_assert( answer , return FALSE );

	N_STR  *input_data = file_to_nstr( _nstr( in ) );
	if( !input_data )
	{
		n_log( LOG_ERR , "could not load input file %s" , _nstr( in ) );
		return FALSE ;
	}

	N_STR *decoded = n_vigenere_decode_qa( input_data , question , answer ); 
	free_nstr( &input_data );

	int ret = nstr_to_file( decoded , _nstr( out ) );
	free_nstr( &decoded );

	return ret ;
}


/*!\fn int n_vigenere_encode_file_qa( N_STR *in , N_STR *out , N_STR *question , N_STR *answer )
 * \brief directly vigenere encode a file using question and answer
 * \param in input path and filename
 * \param out output path and filename
 * \param question question key to use
 * \param answer answer key to use
 * \return TRUE or FALSE
 */
int n_vigenere_encode_file_qa( N_STR *in , N_STR *out , N_STR *question , N_STR *answer )
{
	__n_assert( in , return FALSE );
	__n_assert( out , return FALSE );
	__n_assert( question , return FALSE );
	__n_assert( answer , return FALSE );

	N_STR  *input_data = file_to_nstr( _nstr( in ) );
	if( !input_data )
	{
		n_log( LOG_ERR , "could not load input file %s" , _nstr( in ) );
		return FALSE ;
	}

	N_STR *encoded = n_vigenere_encode_qa( input_data , question , answer ); 
	free_nstr( &input_data );

	int ret = nstr_to_file( encoded , _nstr( out ) );
	free_nstr( &encoded );

	return ret ;
}
