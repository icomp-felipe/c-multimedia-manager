#ifndef _MEDIA_CONVERTER_H_
#define _MEDIA_CONVERTER_H_

#ifndef _LIBMEDIA_H_
#include "libmedia.h"
#endif

#ifndef _DATABASE_DRIVER_H_
#include "databaseDriver.h"
#endif

/** Definição do tamanho máximo de um caminho de arquivo */
#define MEDIA_CONV_PATH_SIZE 2048

extern db_file_pont media_conv_encryption_wizard(tipoArquivo banco, const char* full_path, const char* relative_path, const char* filename_only);
extern db_file_pont media_conv_encrypt          (const char* source_full_path, const char* source_relative_path, const char* target_hashed_filename);
extern boolean      media_conv_decrypt          (const char* file_hash, boolean file_extract_all);

extern boolean media_conv_debugger(const char* file_hash);
extern boolean media_conv_import_wizard(int argc, char** argv);

extern boolean media_conv_valid_file(tipoArquivo file_source);

#endif
