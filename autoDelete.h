/**************************************************************
 *                Gerenciador Multimídia v.4.0
 * 
 * Escrito por: Felipe André <felipeandresouza@hotmail.com>
 * 
 * Biblioteca : autoDelete.h  Última Edição: 11/01/2018
 * 
 **************************************************************
 * 
 * 
 * ************************************************************/

#ifndef _AUTO_DELETE_H_
#define _AUTO_DELETE_H_

#ifndef _LIBMEDIA_H_
#include "libmedia.h"
#endif

extern boolean auto_del_list(void);
extern boolean auto_del_init(void);
extern boolean auto_del_free(void);
extern boolean auto_del_refresh(void);
extern boolean auto_del_is_deletable(const char* filename);

#endif
