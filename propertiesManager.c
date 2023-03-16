/**************************************************************
 *                Gerenciador Multimídia v.4.0
 * 
 * Escrito por: Felipe André <felipeandresouza@hotmail.com>
 * 
 * Biblioteca : props_structManager.c  Última Edição: 11/01/2018
 * 
 **************************************************************
 * 
 * Esta biblioteca é responsável pela inicialização de proprie-
 * dades do sistema.
 * 
 * ************************************************************/

#include "libconfuse/confuse.h"
#include "propertiesManager.h"
#include "stringUtils.h"
#include "fileUtils.h"
#include <pwd.h>

/** Variável que controla a inicialização da struct "cfg_opt_t" */
boolean props_initialized = false;

/** Variável que armazena o caminho do arquivo de propriedades */
char* props_file_path;

/** Estrutura de dados que armazena as propriedades do sistema */
cfg_t* props_struct;

/** Inicialização das propriedades do sistema */
const char* PROP_FILENAME             = "program.properties";
const char* PROP_MAIN_PATH            = "media.default.path";
const char* PROP_MEDIA_PLAYER_REGEX   = "media.player.regex";
const char* PROP_FILE_MANAGER_REGEX   = "media.file.manager.regex";
const char* PROP_MEDIA_EXTRACT_PATH   = "media.output.path";
const char* PROP_MEDIA_CONVERT_REGEX  = "media.conversion.cmd";
const char* PROP_MEDIA_SUPPORTED_LIST = "media.supported.list";
const char* PROP_MEDIA_AUTO_DEL_LIST  = "media.extension.delete";

/** Retorna o diretório padrão de trabalho do software, que fica no diretório do usuário */
char* props_get_file_path(void) {
	
	struct passwd *pw = getpwuid(getuid());
	const char *homedir = pw->pw_dir;
	
	char* path = file_utils_mount_path(3,homedir,"/.media",PROP_FILENAME);
	
	return path;

}

/** Carrega o arquivo de propriedades pra memória */
boolean props_parse(void) {
	
	if (cfg_parse(props_struct,props_file_path) == CFG_PARSE_ERROR) {
		printf(":: Failed to read parameters!\n");
		return false;
	}
	
	return true;	
}

/** Configura as propriedades esperadas pelo sistema */
boolean props_init(void) {
	
	props_file_path = props_get_file_path();
	
	cfg_opt_t opts[] =
	{
		CFG_STR     (PROP_MAIN_PATH           , NULL, CFGF_NONE),
		CFG_STR     (PROP_MEDIA_PLAYER_REGEX  , NULL, CFGF_NONE),
		CFG_STR     (PROP_FILE_MANAGER_REGEX  , NULL, CFGF_NONE),
		CFG_STR     (PROP_MEDIA_EXTRACT_PATH  , NULL, CFGF_NONE),
		CFG_STR     (PROP_MEDIA_CONVERT_REGEX , NULL, CFGF_NONE),
		CFG_STR_LIST(PROP_MEDIA_SUPPORTED_LIST, NULL, CFGF_NONE),
		CFG_STR_LIST(PROP_MEDIA_AUTO_DEL_LIST , NULL, CFGF_NONE),
		CFG_END()
	};
	
	props_struct = cfg_init(opts, CFGF_NONE);
	props_initialized = true;
	
	props_parse();
	
	return true;
}

/** Desaloca a struct de propriedades da memória */
boolean props_free(void) {
	
	cfg_free(props_struct);
	free(props_file_path);
	
	return true;
}

/** Retorna uma propriedade no formato de string identificada por "target".
 *  O parâmetro "realTimeParse" faz o programa ler o arquivo de proprieda-
 *  des na hora da execução desta função.
 *  Obs.: aqui uso o strdup() para retornar a string, logo, não esqueça de,
 *  quando não precisar mais de seu recurso, dar um free() nele :)       */
char* props_get_string(const char* target, boolean realTimeParse) {
	
	if (!props_initialized)
		props_init();
	
	if (realTimeParse)
		props_parse();
	
	short i = 0;
	char* replaced = strdup(cfg_getstr(props_struct,target));
	
	// Trata o caracter ' como "
	while (replaced[i] != '\0') {
		if (replaced[i] == '\'')
			replaced[i] = '"';
		i++;
	}
	
	return replaced;
}

/** Retorna uma propriedade no formato de int identificada por "target".
 *  O parâmetro "realTimeParse" faz o programa ler o arquivo de proprieda-
 *  des na hora da execução desta função. */
int props_get_integer(const char* target, boolean realTimeParse) {
	
	if (!props_initialized)
		props_init();
	
	if (realTimeParse)
		props_parse();
	
	return cfg_getint(props_struct,target);
}

/** Retorna o tamanho de uma lista de propriedades */
size_t props_get_list_size(const char* target, boolean realTimeParse) {
	
	if (!props_initialized)
		props_init();
	
	if (realTimeParse)
		props_parse();

	return cfg_size(props_struct,target);
	
}

/** Retorna uma lista de propriedades */
nodeStringList props_get_string_list(const char* target, boolean realTimeParse) {
	
	if (!props_initialized)
		props_init();
	
	if (realTimeParse)
		props_parse();
	
	size_t i, size = cfg_size(props_struct,target);
	char**    data = (char**) malloc(size * sizeof(char**));
	
	if (!data) {
		perror(":: Falha ao alocar vetor de string!\n");
		return NULL;
	}
	
	for (i=0; i<size; i++)
		data[i] = strdup(cfg_getnstr(props_struct,target,i));

	return str_utils_list_create(size,data);
	
}
