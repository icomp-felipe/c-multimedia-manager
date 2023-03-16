/*************************************************************
 *                Gerenciador Multimídia v.3.5
 * 
 * Escrito por: Felipe André <felipeandresouza@hotmail.com>
 * 
 * Biblioteca: commons.h          Última Edição: 23/05/2016
 * 
 *************************************************************
 * 
 * Aqui você encontra as funções comuns a grande parte do sis-
 * tema, tais como operações com string, arquivo, streams...
 * 
 * ***********************************************************/

#ifndef _COMMONS_H_
#define _COMMONS_H_

#ifndef _LIBMEDIA_H_
#include "libmedia.h"
#endif

#ifndef _STRING_UTILS_H_
#include "stringUtils.h"
#endif

#define x86 1
#define x86_64 2


int getRandomIndex(int maxIndex);

tipoArquivo abreBanco (const char* mode);


extern int	   com_println(const char* format, ...);
extern int	com_perrorln(const char* format, ...);
extern int	   com_fprintln(tipoArquivo stream, const char* format, ...);
extern boolean com_enable_special_characters(void);
extern boolean com_screen_clear(void);
extern int     com_get_cpu_architecture(void);
extern boolean com_free_all(unsigned short int argc, ...);
extern boolean com_flush_stdin(void);
extern boolean com_pause(void);
extern int     com_print_version(void);
extern boolean com_extension_contains_in_list(const char* filename, nodeStringList list);

#endif
