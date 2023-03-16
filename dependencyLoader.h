/**************************************************************
 *                Gerenciador Multimídia v.3.5
 * 
 * Escrito por: Felipe André <felipeandresouza@hotmail.com>
 * 
 * Biblioteca : dependencyLoader.h  Última Edição: 23/05/2016
 * 
 **************************************************************
 * 
 * Esta biblioteca é responsável pela inicialização e verifi-
 * cação dos arquivos e diretórios do sistema.
 * 
 * ************************************************************/

#ifndef _DEPENDENCY_LOADER_H_
#define _DEPENDENCY_LOADER_H_

#ifndef _LIBMEDIA_H_
#include "libmedia.h"
#endif

/** Definição das constantes para utilização na função 'dep_loader_get()' */
#define DEP_MAIN_PATH          1
#define DEP_MEDIA_STORAGE_PATH 2
#define DEP_MEDIA_EXTRACT_PATH 3
#define DEP_MEDIA_PLAYER_REGEX 4
#define DEP_FILE_MANAGER_REGEX 5
#define DEP_DATABASE_MAIN_PATH 6
#define DEP_DATABASE_TEMP_PATH 7
#define DEP_DATABASE_BKPS_PATH 8
#define DEP_PASSWORD_FILE_PATH 9

/** Definição do tipo de dados a ser utilizado na função 'dep_loader_get()' */
typedef unsigned short int dep_type;

extern const char* dep_loader_get(dep_type resource);

extern boolean dep_loader_init(void);
extern boolean dep_loader_free(void);
extern boolean dep_loader_refresh(void);
extern boolean dep_loader_self_test(void);

#endif
