/**************************************************************
 *                Gerenciador Multimídia v.4.0
 * 
 * Escrito por: Felipe André <felipeandresouza@hotmail.com>
 * 
 * Biblioteca : fileUtils.h  Última Edição: 11/01/2018
 * 
 **************************************************************
 * 
 * 
 * ************************************************************/

#ifndef _FILE_UTILS_H_
#define _FILE_UTILS_H_

#ifndef _LIBMEDIA_H_
#include "libmedia.h"
#endif

extern boolean file_utils_isOK(tipoArquivo file);
extern boolean file_utils_isnOK(unsigned short int files_count, ...);

extern boolean file_utils_is_not_OK (tipoArquivo file);
extern boolean file_utils_isn_not_OK(unsigned short int files_count, ...);

extern char* file_utils_get_current_dir  (void);
extern char* file_utils_extract_extension(const char* filename);
extern char* file_utils_extract_filename (const char* filename);

extern int file_utils_close(tipoArquivo file);
extern int file_utils_end_forward(tipoArquivo file);

extern char* file_utils_parse_path(char* path);
extern char* file_utils_mount_path(size_t elements, ...);

extern boolean file_utils_exists(const char* filename);
extern size_t file_utils_size(tipoArquivo arquivo);

extern boolean file_utils_parse_file(const char* filename);
extern boolean file_utils_parse_dir (const char* dir_name);

extern boolean file_utils_copy(tipoArquivo file_source, tipoArquivo file_target, size_t file_source_size);
extern boolean file_utils_copy_progress_bar(tipoArquivo file_source, tipoArquivo file_target, size_t file_source_size, const char* progressBarTitle);
extern void file_utils_list_dir(char* dir_path);

extern char* file_utils_prepare_relative_path(const char* full_path, const char* path_to_remove);
extern boolean file_utils_is_file(const char* path);
extern boolean file_utils_is_dir (const char* path);

extern char* file_utils_remove_extension(const char* filename);
extern tipoArquivo file_utils_touch(const char* full_path);

extern char* file_utils_readable_file_size(size_t file_size);
extern uint32_t file_utils_dir_count(const char* directory_path);

#endif
