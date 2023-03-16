/**************************************************************
 *                Gerenciador Multimídia v.4.0
 * 
 * Escrito por: Felipe André <felipeandresouza@hotmail.com>
 * 
 * Biblioteca : supportedMedia.h  Última Edição: 11/01/2018
 * 
 **************************************************************
 * 
 * 
 * ************************************************************/

#ifndef _SUPPORTED_MEDIA_H_
#define _SUPPORTED_MEDIA_H_

#ifndef _LIBMEDIA_H_
#include "libmedia.h"
#endif

extern boolean sup_media_list(void);
extern boolean sup_media_init(void);
extern boolean sup_media_free(void);
extern boolean sup_media_refresh(void);
extern boolean sup_media_is_supported(const char* filename);

#endif
