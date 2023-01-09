/**\file n_kafka.h
 * kafka generic produce and consume event header
 *\author Castagnier Mickael
 *\version 1.0
 *\date 23/11/2022
 */

#ifndef __N_KAFKA
#define __N_KAFKA

#ifdef __cplusplus
extern "C"
{
#endif

/**\defgroup N_KAFKA KAFKA: generic event producer and consumer functions
  \addtogroup N_KAFKA
  @{
  */

#include "nilorea/n_log.h"
#include "nilorea/n_network.h"
#include "nilorea/n_network.h"
#include "cJSON.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <libgen.h>
#include <errno.h>

#ifdef __N_LOCAL_LIBRDKAFKA
#include "librdkafka/src/rdkafka.h"
#else
#include <librdkafka/src/rdkafka.h>
#endif

/*! structure of a KAFKA message */
typedef struct N_KAFKA_MESSAGE	
{
	/*! string containing the message */
	N_STR *event_string ;
	/*! id of the generated message */
	unsigned int event_id ;
} N_KAFKA_MESSAGE ;

/*! structure of a KAFKA consumer or producer handle */
typedef struct N_KAFKA
{
	/*! list of N_KAFKA_MESSAGE to send */
	LIST *events_to_send ;
	/*! list of received N_KAFKA_MESSAGE */
	LIST *received_event ;
	/*! next new N_KAFKA_MESSAGE id value */
	unsigned int current_event_id ;
	/*! kafka structure handle */
	rd_kafka_conf_t *rd_kafka_conf ;
	/*! lock for the N_KAFKA handle */
	pthread_mutex_t lock ;
	/*! KAFKA handle mode: 0 -> consumer , 1 -> producer */
	int mode ;
} N_KAFKA ;

// allocate a new kafka structure from file, launching a thread for the pooler
N_KAFKA *n_kafka_new_from( char *configfile , int mode );
// retry = -1 unlimited, 0 no retry, >0 number of retries 
int n_kafka_send_event( N_KAFKA *rdkafka , N_STR *event , int retrynb );
// try to read an event 
int n_kafka_read_event( N_KAFKA *rdkafka , N_STR **event );
// close , free, stop the managing thread
int n_kafka_close( N_KAFKA **rdkafka );

/**
  @}
  */

#ifdef __cplusplus
}
#endif

#endif // header guard

