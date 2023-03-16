/*************************************************************
 *                Gerenciador Multimídia v.4.0
 * 
 * Escrito por: Felipe André <felipeandresouza@hotmail.com>
 * 
 * Biblioteca: commons.c          Última Edição: 24/01/2018
 * 
 *************************************************************
 * 
 * Aqui você encontra as funções comuns a grande parte do sis-
 * tema, tais como operações com string, arquivo, streams...
 * 
 * ***********************************************************/

#include "libmedia.h"
#include "fileUtils.h"
#include "stringUtils.h"
#include "dependencyLoader.h"

/** Variação da função "printf", pula uma linha ao exibir a mensagem */
int	com_println(const char* format, ...) {
	
	int status;
 	va_list args;
 	
 	va_start(args, format);
 	status = vfprintf(stdout, format, args);
 	fputc(0xA,stdout);	fflush(stdout);
 	va_end(args);
 	
 	return status;
}

/** Variação da função "fprintf", pula uma linha ao exibir a mensagem */
int	com_fprintln(tipoArquivo stream, const char* format, ...) {
	
	int status;
 	va_list args;
 	
 	va_start(args, format);
 	status = vfprintf(stream, format, args);
 	fputc(0xA,stream);	fflush(stream);
 	va_end(args);
 	
 	return status;
}

/** Variação da função "perror", pula uma linha ao exibir a mensagem */
int	com_perrorln(const char* format, ...) {
	
	int status;
 	va_list args;
 	
 	va_start(args, format);
 	status = vfprintf(stderr, format, args);
 	fputc(0xA,stderr);	fflush(stderr);
 	va_end(args);
 	
 	return status;
}

/** Carrega os acentos para correta visualização
 *  de caracteres especiais nos sistemas Windows */
boolean com_enable_special_characters(void) {
	
	#ifdef WIN32
		setlocale(LC_ALL,"");
	#endif
	
	return true;
}

/** Limpa a tela */
boolean com_screen_clear(void) {
	
	#ifdef __linux__
		system("clear");
	#elif defined WIN32
		system("cls");
	#endif
	
	return true;
	
}

/** Verifica a arquitetura do processador */
int com_get_cpu_architecture(void) {
    return (__SIZEOF_POINTER__ == 4) ? x86 : x86_64;
}

/** Libera "argc" ponteiros da memória */
boolean com_free_all(unsigned short int argc, ...) {
	
	va_list argv;

	va_start(argv,argc);
	
	for (;argc;argc--) {
		
		void* pointer = va_arg(argv,void*);
		
		if (pointer != NULL)
			free(pointer);
		
	}
	
	va_end(argv);
	
	return true;
}

/** Limpa o buffer do teclado */
boolean com_flush_stdin(void) {
	
	stdin = freopen(NULL,"r",stdin);
	
	return true;
}

/** Aplica uma pausa no sistema */
boolean com_pause(void) {
	
	com_flush_stdin();
	getchar();
	
	return true;
}

/** Imprime a versão atual do sistema */
int com_print_version(void) {
	return com_println("=====| Gerenciador Multimídia v.%s |=====\n",PROGRAM_VERSION);
}

/** Verifica se a extensão do arquivo informado consta em 'list' */
boolean com_extension_contains_in_list(const char* filename, nodeStringList list) {

	const char* extension = file_utils_extract_extension(filename);

	return str_utils_list_contains(extension,list,false);
	
}





// MANIPULACAO DE STRINGS

/** Abre o arquivo de Banco de Dados */
tipoArquivo abreBanco(const char* mode) {
	
	tipoArquivo arquivo = fopen(dep_loader_get(DEP_DATABASE_MAIN_PATH),mode);
	
	if (file_utils_is_not_OK(arquivo))
		com_println("x Falha ao abrir o arquivo de banco de dados!");
		
	return arquivo;
}
