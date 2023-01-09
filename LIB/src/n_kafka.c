/**\file n_kafka.c
 *  generic kafka consume and produce event functions
 *\author Castagnier Mickael
 *\version 1.0
 *\date 23/11/2022
 */

#include "nilorea/n_kafka.h"

char *config_file = NULL ,
	 *event_string = NULL ,
	 *event_file = NULL ,
	 *kafka_topic = NULL ,
	 *event_log_file = NULL ,
	 *log_prefix = NULL ;

int log_level = LOG_ERR ,  /* default log level */
	getoptret = 0 ,		   /* getopt return value */
	timeout = 30000 ,      /* default timeout */
	got_kafka_answer = 0 ; /* count messages from kafka */

/**
 * @brief Message delivery report callback.
 *
 * This callback is called exactly once per message, indicating if
 * the message was succesfully delivered
 * (rkmessage->err == RD_KAFKA_RESP_ERR_NO_ERROR) or permanently
 * failed delivery (rkmessage->err != RD_KAFKA_RESP_ERR_NO_ERROR).
 *
 * The callback is triggered from rd_kafka_poll() and executes on
 * the application's thread.
 */
static void n_kafka_delivery_message_callback( rd_kafka_t *rk , const rd_kafka_message_t *rkmessage , void *opaque )
{
	//opaque is unused at the moment
	(void)opaque;

	__n_assert( rk , n_log( LOG_ERR , "rk=NULL is not a valid kafka handle" ); return ; }
	__n_assert( rkmessage , n_log( LOG_ERR , "rkmessage=NULL is not a valid kafka message" ); return ; }

	if (rkmessage->err)
		n_log( LOG_ERR , "%s message delivery failed: %s" , log_prefix , rd_kafka_err2str( rkmessage -> err) );
	else
		n_log( LOG_DEBUG , "%s message delivered (%ld bytes, partition %d)" , log_prefix , rkmessage -> len , rkmessage -> partition );

	/* The rkmessage is destroyed automatically by librdkafka */
	got_kafka_answer = 1 ;
}

			case 's':
				Malloc( event_string , char , strlen( optarg ) + 6 );
				event_string[ 0 ] = 0 ;
				strcpy( event_string + 5 , optarg );
				break ;

	set_log_level( log_level );
	if( event_log_file )
	{
		int log_file_ret = set_log_file( event_log_file );
		n_log( LOG_DEBUG , "%s log to file: %s , %d  , %p" , log_prefix , event_log_file , log_file_ret , get_log_file() );
	}

	/* testing parameters */
	if( !config_file )
	{
		n_log( LOG_ERR , "%s parameter config_file needs to be set !" , log_prefix );
		exit( 1 );
	}
	if( !event_string && !event_file )
	{
		n_log( LOG_ERR , "%s one of (event_string|event_file) needs to be set !" , log_prefix );
		exit( 1 );
	}

	if( event_string && event_file )
	{
		n_log( LOG_ERR , "%s do not define event_string AND event_file, only one needs to be set !" , log_prefix );
		exit( 1 );
	}

	if( event_file )
	{
		N_STR *event_file_str = file_to_nstr( event_file );
		event_string = event_file_str -> data ;
		event_file_str -> data = NULL ;
		free_nstr( &event_file_str ); 
	}

	// initialize kafka object
	size_t errstr_len = 1024 ;
	char* errstr = NULL ; Malloc( errstr , char , errstr_len ); if( !errstr ) exit( 1 ); //passed to rd_kafka_*funcs
	rd_kafka_conf_t *rd_kafka_conf = rd_kafka_conf_new();

	// load config file
	N_STR *config_string = NULL ;
	config_string = file_to_nstr( config_file );
	if( !config_string )
	{
		n_log( LOG_ERR , "%s unable to read config from file %s !" , log_prefix , config_file );
		exit( 1 );
	}
	cJSON *json = NULL ;
	json = cJSON_Parse( _nstrp( config_string ) );
	if( !json )
	{
		n_log( LOG_ERR , "%s unable to parse json %s" , log_prefix , config_string );
		exit( 1 );
	}

	int jsonIndex;
	for (jsonIndex = 0; jsonIndex < cJSON_GetArraySize(json); jsonIndex++) 
	{
		cJSON *entry = cJSON_GetArrayItem(json, jsonIndex);

		__n_assert( entry , continue );
		__n_assert( entry -> string , continue );
		__n_assert( entry -> valuestring , continue );

		if( entry -> string[ 0 ] != '-' )
		{
			if( strcmp( "topic" , entry -> string ) != 0 && strcmp( "value.schema.id" , entry -> string ) != 0 )
			{
				if( rd_kafka_conf_set( rd_kafka_conf , entry -> string , entry -> valuestring , errstr , errstr_len ) != RD_KAFKA_CONF_OK )
				{
					n_log( LOG_ERR , "%s kafka config: %s" , log_prefix , errstr );
					exit( 1 );
				}
				else
				{
					n_log( LOG_DEBUG , "kafka config enabled: %s => %s" , entry -> string , entry -> valuestring );
				}
			}
		}
		else
		{
			n_log( LOG_DEBUG , "kafka config disabled: %s => %s" , entry -> string , entry -> valuestring );
		}
	}

	cJSON *jstr = NULL ;
	
	jstr = cJSON_GetObjectItem( json, "topic" );
	if( jstr && jstr -> valuestring )
	{
		kafka_topic = strdup( jstr -> valuestring );
		n_log( LOG_DEBUG , "kafka topic: %s" , kafka_topic );
	}
	else
	{
		n_log( LOG_ERR , "no topic configured !" );
		exit( 1 );
	}

	jstr = cJSON_GetObjectItem( json, "value.schema.id" );
	if( jstr && jstr -> valuestring  )
	{
		int schem_v = atoi( jstr -> valuestring );
		if( schem_v < 0 || schem_v > 9999 )
		{
			n_log( LOG_ERR , "%s invalid schema id %d"  , log_prefix , schem_v );
			exit( 1 );
		}
		/*event_string[ 1 ] = schem_v / 1000 ;
		  schem_v = schem_v - 1000 * event_string[ 1 ];
		  event_string[ 2 ] = schem_v / 100 ;
		  schem_v = schem_v - 100 * event_string[ 2 ];
		  event_string[ 3 ] = schem_v /10 ;
		  schem_v = schem_v - 10 * event_string[ 3 ];
		  event_string[ 4 ] = schem_v ;*/
		int32_t schema_id = htonl( schem_v );
		memcpy( (void *)&event_string[ 1 ] , &schema_id , sizeof( int32_t ) ); 
		n_log( LOG_DEBUG , "%s value.schema.id:%d" , log_prefix , schema_id );
	}

	// set delivery callback
	rd_kafka_conf_set_dr_msg_cb( rd_kafka_conf , dr_msg_cb );

	// create the producer
	rd_kafka_t* rd_kafka_producer = rd_kafka_new( RD_KAFKA_PRODUCER , rd_kafka_conf , errstr, errstr_len );
	if( !rd_kafka_producer )
	{
		n_log( LOG_ERR , "%s failed to create new producer: %s" , log_prefix , errstr );
		exit( 1 );
	}

	// free errstr as it's no used anymore
	Free( errstr );

	n_log( LOG_DEBUG , "%s event_string: %s" , log_prefix , _str( event_string + 5 ) );

	// Create topic object
	rd_kafka_topic_t* rd_kafka_topic = rd_kafka_topic_new( rd_kafka_producer , kafka_topic , NULL );

retry:

	if( rd_kafka_produce( rd_kafka_topic , RD_KAFKA_PARTITION_UA , RD_KAFKA_MSG_F_FREE , event_string , strlen( event_string + 5 ) + 6 , NULL , 0 , NULL ) == -1 )
	{
		// Failed to *enqueue* message for producing.
		n_log( LOG_ERR , "%s failed to produce to topic %s: %s" , log_prefix , rd_kafka_topic_name( rd_kafka_topic ) , rd_kafka_err2str(rd_kafka_last_error() ) );

		if( rd_kafka_last_error() != 0 )
		{
			// wait 1000 msecs for kafka response
			rd_kafka_poll( rd_kafka_producer , 1000 );
			goto retry;
		}
	}
	// Success to *enqueue* message for producing
	int kafka_ret = 0 ;

	n_log( LOG_NOTICE , "%s producer success to enqueue message (%ld byte) for topic %s" , log_prefix , strlen( event_string + 5 ), rd_kafka_topic_name( rd_kafka_topic ) );

	rd_kafka_poll( rd_kafka_producer , timeout );
	if( got_kafka_answer == 0 )
	{
		n_log( LOG_ERR , "%s producer did got an answer from kafka %s for message: %s" , log_prefix , rd_kafka_topic_name( rd_kafka_topic ) , event_string + 5 );
		kafka_ret = 1 ;
	}
	rd_kafka_topic_destroy(rd_kafka_topic);
	rd_kafka_destroy(rd_kafka_producer);

	return kafka_ret ;
}
