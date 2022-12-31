/**\file n_crypto.h
 *  vigenere encoding and decoding functions using N_STR/files
 *\author Castagnier Mickael
 *\version 1.0
 *\date 10/11/2022
 */

#ifndef __N_CRYPTO_H
#define __N_CRYPTO_H

#ifdef __cplusplus
extern "C"
{
#endif

/**\defgroup CYPHER_VIGENERE CYPHERS: VIGENERE (encode / decode) a (N_STR / file) with a root key or a combination of a question and an answer
 * \addtogroup CYPHER_VIGENERE
  @{
  */

#include <stdlib.h>
#include <memory.h>
#include "nilorea/n_str.h"

/*! The % operator returns a result that adopts the sign of the dividend, a true mathematical modulus adopts the sign of the divisor. This method implements a mathematical modulus */
#define n_mathmod( __a, __b ) ( (__a % __b + __b) % __b )

N_STR *n_vigenere_get_rootkey( size_t rootkey_size );
N_STR *n_vigenere_get_question( size_t question_size );
N_STR *n_vigenere_get_answer( N_STR *root_key , N_STR *question );
N_STR *n_vigenere_encode( N_STR *string , N_STR *key );
N_STR *n_vigenere_decode( N_STR *string , N_STR *key );
N_STR *n_vigenere_quick_encode( N_STR *decoded_question );
N_STR *n_vigenere_quick_decode( N_STR *encoded_question );
N_STR *n_vigenere_encode_qa( N_STR *input_data , N_STR *question , N_STR *answer );
N_STR *n_vigenere_decode_qa( N_STR *input_data , N_STR *question , N_STR *answer );
int n_vigenere_encode_file( N_STR *in , N_STR *out , N_STR *rootkey );
int n_vigenere_decode_file( N_STR *in , N_STR *out , N_STR *rootkey );
int n_vigenere_decode_file_qa( N_STR *in , N_STR *out , N_STR *question , N_STR *answer );
int n_vigenere_encode_file_qa( N_STR *in , N_STR *out , N_STR *question , N_STR *answer );

/**
@}
*/

#ifdef __cplusplus
}
#endif
/* #ifndef __N_CRYPTO_H */
#endif
