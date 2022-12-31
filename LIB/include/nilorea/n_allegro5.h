/**\file n_allegro5.h
 * Allegro5 helpers
 *\author Castagnier Mickael
 *\version 1.0
 *\date 10/05/2020
 */

#ifndef __NILOREA_ALLEGRO5_HELPER__
#define __NILOREA_ALLEGRO5_HELPER__

#ifdef __cplusplus
extern "C"
{
#endif

/**\defgroup ALLEGRO5 ALLEGRO5: allegro 5 helpers (user input,...)
   \addtogroup ALLEGRO5
  @{
  */

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_ttf.h>

int get_keyboard( ALLEGRO_USTR *str, ALLEGRO_EVENT event );

/**
  @}
  */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __NILOREA_ALLEGRO5_HELPER__ */
