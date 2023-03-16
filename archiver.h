/**************************************************************
 *                Gerenciador Multimídia v.4.0
 * 
 * Escrito por: Felipe André <felipeandresouza@hotmail.com>
 * 
 * Biblioteca : archiver.h  Última Edição: 11/01/2018
 * 
 **************************************************************
 * 
 * 
 * ************************************************************/

#ifndef _ARCHIVER_H_
#define _ARCHIVER_H_

#ifndef _LIBMEDIA_H_
#include "libmedia.h"
#endif

#ifndef _DATABASE_DRIVER_H_
#include "databaseDriver.h"
#endif

enum ARCH_STATUS {ARCH_FILE_DELETED ,
                  ARCH_FILE_IGNORED ,
                  ARCH_FILE_IMPORTED,
                  ARCH_FILE_FAILED  ,
                  ARCH_DIR_FAILED   ,
                  ARCH_DIR_IMPORTED ,
                  ARCH_PATH_FAILED
};

extern void arch_import_preloader(tipoArquivo banco, const char* path_to_import, const char* internal_path_reference, unsigned short int* status);
extern boolean arch_conv_import_wizard(int argc, char** argv);
extern db_file_pont media_conv_encryption_wizard(tipoArquivo banco, const char* full_path, const char* relative_path, const char* filename_only);
extern boolean arch_extract_all(void);
extern boolean arch_media_delete(const char* hash);

#endif
