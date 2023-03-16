/**************************************************************
 *                Gerenciador Multimídia v.3.5
 * 
 * Escrito por: Felipe André <felipeandresouza@hotmail.com>
 * 
 * Biblioteca : keyManager.h      Última Edição: 23/05/2016
 * 
 **************************************************************
 * 
 * Esta biblioteca é responsável pelas operações com o arquivo
 * de senhas do sistema.
 * 
 * ************************************************************/

#ifndef _KEY_MANAGER_H_
#define _KEY_MANAGER_H_

#ifndef _LIBMEDIA_H_
#include "libmedia.h"
#endif

extern boolean passwd_man_update_key_wizard(void);
extern boolean passwd_man_parse_wizard     (void);

#endif
