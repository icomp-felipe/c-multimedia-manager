/**************************************************************
 *                Gerenciador Multimídia v.4.0
 * 
 * Escrito por: Felipe André <felipeandresouza@hotmail.com>
 * 
 * Biblioteca : propertiesManager.h  Última Edição: 11/01/2018
 * 
 **************************************************************
 * 
 * Esta biblioteca é responsável pelas operações de reparo da
 * base de dados.
 * 
 * ************************************************************/

#ifndef _PROPERTIES_MANAGER_H_
#define _PROPERTIES_MANAGER_H_

#ifndef _LIBMEDIA_H_
#include "libmedia.h"
#endif

#ifndef _STRING_UTILS_H_
#include "stringUtils.h"
#endif

/** Definição das constantes de propriedades */
extern const char* PROP_FILENAME;
extern const char* PROP_MAIN_PATH;
extern const char* PROP_MEDIA_PLAYER_REGEX;
extern const char* PROP_FILE_MANAGER_REGEX;
extern const char* PROP_MEDIA_EXTRACT_PATH;
extern const char* PROP_MEDIA_CONVERT_REGEX;
extern const char* PROP_MEDIA_SUPPORTED_LIST;
extern const char* PROP_MEDIA_AUTO_DEL_LIST;

extern boolean props_free(void);
extern int     props_get_integer  (const char* target, boolean realTimeParse);
extern char*   props_get_string   (const char* target, boolean realTimeParse);
extern size_t  props_get_list_size(const char* target, boolean realTimeParse);
extern nodeStringList props_get_string_list(const char* target, boolean realTimeParse);

#endif
