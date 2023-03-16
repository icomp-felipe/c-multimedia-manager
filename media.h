/**************************************************************
 *                Gerenciador Multimídia v.3.5
 * 
 * Escrito por: Felipe André <felipeandresouza@hotmail.com>
 * 
 * Biblioteca : media.h             Última Edição: 23/05/2016
 * 
 **************************************************************
 * 
 * Esta biblioteca é a interface com o programa principal con-
 * tido em "media.c", apenas contém a definição das funções u-
 * tilizadas por este.
 * 
 * ************************************************************/

#ifndef _MEDIA_H_
#define _MEDIA_H_

#ifndef _LIBMEDIA_H_
#include "libmedia.h"
#endif

#ifndef _USER_GUI_H_
#include "userGUI.h"
#endif

#ifndef _ARCHIVER_H_
#include "archiver.h"
#endif

/** Funções de controle do programa principal */
extern boolean com_enable_special_characters(void);

#endif
