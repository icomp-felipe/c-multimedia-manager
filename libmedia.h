/**************************************************************
 *                Gerenciador Multimídia v.3.5
 * 
 * Escrito por: Felipe André <felipeandresouza@hotmail.com>
 * 
 * Biblioteca : libmedia.h        Última Edição: 23/05/2016
 * 
 **************************************************************
 * 
 * Esta é uma das bibliotecas mais importantes do sistema, con-
 * tém a definição de todos os tipos e estruturas de dados uti-
 * lizadas pelo software, bem como todas as constantes pré-pro-
 * cessadas e pós-processadas. Também concentra a chamada às
 * outras bibliotecas do C apenas neste arquivo.
 * 
 * ************************************************************/

#ifndef _LIBMEDIA_H_
#define _LIBMEDIA_H_

#define PROGRAM_VERSION "4.0"

/** Importação das bibliotecas necessárias ao programa */

#ifndef _STDIO_H_
#include <stdio.h>
#endif

#ifndef _STDLIB_H_
#include <stdlib.h>
#endif

#ifndef _STRING_H_
#include <string.h>
#endif

#ifndef _STDARG_H_
#include <stdarg.h>
#endif

#ifndef _STDINT_H_
#include <stdint.h>
#endif

#ifndef _CTYPE_H_
#include <ctype.h>
#endif

#ifndef _UNISTD_H_
#include <unistd.h>
#endif

#ifndef _TIME_H_
#include <time.h>
#endif

#ifndef _DIRENT_H_
#include <dirent.h>
#endif

#ifndef _ERRNO_H_
#include <errno.h>
#endif

#ifndef _LOCALE_H_
#include <locale.h>
#endif

#ifndef _STAT_H_
#include <sys/stat.h>
#endif

/** Definição dos limites de tamanho da senha do sistema */

#define MAX_PWD 256

/** Definição das constantes de controle do menu programa */

#define EXIT   EXIT_SUCCESS
#define FIRST  0x1
#define SECOND 0x2
#define THIRD  0x3
#define FOURTH 0x4

/** Definição das constantes funcionais do programa */

#define EQUALS  0x2
#define PATHMAX 512
#define STRMAX  0xFF
#define UNIT 	0x1
#define BUFFER  4096

/** Definição das constantes de validação de arquivo */

#define isValid  1
#define notValid 0

/** Definição das constantes de verificação de arquivo e diretório */

#define isDir  0x4
#define isFile 0x8

/** Implementação do tipo "boolean" em C */

#define true  1
#define false 0

/** Definição das arquiteturas de processador suportadas */

#define x86 1
#define x86_64 2


/** Definição de tipos de dados úteis ao sistema */

/** Estrutura de dados padrão de E/S */
typedef FILE* tipoArquivo;
typedef unsigned short int boolean;





struct indice {
	char hash[STRMAX];
	char filename[STRMAX];
};

typedef struct indice  tipoIndice;
typedef struct indice* nodeIndice;

typedef DIR* 		   tipoDirectory;
typedef struct dirent* nodeDirectory;

#endif
