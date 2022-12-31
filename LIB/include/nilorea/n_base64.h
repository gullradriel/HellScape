/**\file n_base64.h
 *  base64 encoding and decoding functions using N_STR
 *\author Castagnier Mickael
 *\version 1.0
 *\date 10/11/2022
 */

#ifndef __N_BASE64_H
#define __N_BASE64_H

#ifdef __cplusplus
extern "C"
{
#endif

/**\defgroup CYPHER_BASE64 CYPHERS: BASE64 (encode / decode) (from / to) a N_STR *string
  \addtogroup CYPHER_BASE64
  @{
  */

#include <stdlib.h>
#include <memory.h>
#include "nilorea/n_str.h"

bool n_isupper( char c );
bool n_islower( char c );
bool n_isalpha( char c );
char n_toupper( char c );
char n_tolower( char c );

N_STR *n_base64_encode( N_STR *string );
N_STR *n_base64_decode( N_STR *bufcoded );

/**
@}
*/

#ifdef __cplusplus
}
#endif
/* #ifndef __N_BASE64_H */
#endif
