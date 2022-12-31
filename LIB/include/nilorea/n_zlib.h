/*!\file n_zlib.h
 * ZLIB compression handler
 *\author Castagnier Mickael
 *\version 1.0
 *\date 27/06/2017
 */

#ifndef __N__Z_LIB_HEADER
#define __N__Z_LIB_HEADER

#ifdef __cplusplus
extern "C"
{
#endif

#include <zlib.h>
#include "n_str.h"

/**\defgroup ZLIB ZLIB: shortcuts to easy compress/decompress data
  \addtogroup ZLIB
  @{
  */

int GetMaxCompressedLen( unsigned int nLenSrc );
int CompressData(   unsigned char *abSrc, unsigned int nLenSrc, unsigned char *abDst, unsigned int nLenDst );
int UncompressData( unsigned char *abSrc, unsigned int nLenSrc, unsigned char *abDst, unsigned int nLenDst );

N_STR *zip_nstr( N_STR *src );
N_STR *unzip_nstr( N_STR *src );

/**
@}
*/

#ifdef __cplusplus
}
#endif

#endif // header guard
