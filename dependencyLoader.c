/**************************************************************
 *                Gerenciador Multimídia v.4.0
 * 
 * Escrito por: Felipe André <felipeandresouza@hotmail.com>
 * 
 * Biblioteca : dependencyLoader.c  Última Edição: 03/01/2018
 * 
 **************************************************************
 * 
 * Esta biblioteca é responsável pela inicialização e verifi-
 * cação dos arquivos e diretórios do sistema.
 * 
 * ************************************************************/

#include "commons.h"
#include "libmedia.h"
#include "propertiesManager.h"
#include "supportedMedia.h"
#include "autoDelete.h"
#include "fileUtils.h"
#include "dependencyLoader.h"
#include "userGUI.h"

/** Definição dos nomes de arquivos de recursos do programa */
#define DEP_DB_FILENAME       "database.dbx"
#define DEP_DB_BKP_FILENAME   "database.bkp"
#define DEP_DB_TMP_FILENAME   "database.tmp"
#define DEP_PASSWORD_FILENAME "password.key"

#define DEP_STORAGE_PATH      "Media"

/** Estes ponteiros guardam os caminhos absolutos
 *  dos arquivos e diretórios da aplicação */
char* main_path;
char* media_storage_path;
char* media_extract_path;
char* media_player_regex;
char* file_manager_regex;
char* database_main_path;
char* database_temp_path;
char* database_bkps_path;
char* password_file_path;

/** Recupera um recurso */
const char* dep_loader_get(dep_type resource) {
	
	switch(resource) {
		
		case DEP_MAIN_PATH:
			return main_path;
			
		case DEP_MEDIA_STORAGE_PATH:
			return media_storage_path;
			
		case DEP_MEDIA_EXTRACT_PATH:
			return media_extract_path;
			
		case DEP_MEDIA_PLAYER_REGEX:
			return media_player_regex;
			
		case DEP_DATABASE_MAIN_PATH:
			return database_main_path;
		
		case DEP_DATABASE_TEMP_PATH:
			return database_temp_path;
		
		case DEP_DATABASE_BKPS_PATH:
			return database_bkps_path;
		
		case DEP_PASSWORD_FILE_PATH:
			return password_file_path;
		
		case DEP_FILE_MANAGER_REGEX:
			return file_manager_regex;
		
	}
	
	return NULL;
	
}

/** Verifica se os arquivos e diretórios do programa estão ok */
boolean dep_loader_self_test(void) {
	
	boolean password_exists;
	
	com_println("=====| Gerenciador Multimídia v.4.0 |=====\n");
	
	printf("* Verificando arquivo de senha...");	
	password_exists = file_utils_parse_file(password_file_path);
	
	printf("* Verificando arquivo de banco de dados...");
	file_utils_parse_file(database_main_path);
	
	printf("* Verificando diretorio de mídia...");
	file_utils_parse_dir(media_storage_path);
	
	printf("* Verificando diretório de saída de mídia...");
	file_utils_parse_dir(media_extract_path);
	
	if (!password_exists) {
		com_screen_clear();
		com_println(":: O arquivo de senhas não foi encontrado! Criando um novo...");
		user_gui_password_manager();
	}
	
	com_println(NULL);
	
	return password_exists;
}

/** Apenas imprime os diretórios e arquivos do
 *  sistema para fins de conferência e debug */
void dep_loader_list() {
	
	com_print_version();
	
	com_println("=====> Listagem de Arquivos e Diretórios <=====\n");
	com_println("==> Diretórios Principais do Sistema\n");
	com_println("* Raiz do Programa      : %s",main_path);
	com_println("* Armazenamento de Mídia: %s",media_storage_path);
	com_println("* Extração de Mídia     : %s",media_extract_path);

	com_println(NULL);
	com_println("==> Arquivos de Banco de Dados\n");
	com_println("* Principal : %s",database_main_path);
	com_println("* Temporário: %s",database_temp_path);
	com_println("* Backup    : %s",database_bkps_path);
	
	com_println(NULL);
	com_println("==> Arquivos de Segurança\n");
	com_println("* Senha do Sistema: %s",password_file_path);

	com_println(NULL);
	com_println("==> Outros\n");
	com_println("* Linha de Comando do Media Player : %s",media_player_regex);
	com_println("* Linha de Comando do File Manager : %s",file_manager_regex);

	sup_media_list();
	auto_del_list ();

	com_pause();

}

/** Inicializa os ponteiros globais com os valores lidos do arquivo
 *  de propriedades, este método pode ser executado mais de uma vez
 *  durante a execução do programa. */
boolean dep_loader_init_dynamic(boolean realTimeUpdate) {
	
	// Diretório de saída de mídia
	free(media_extract_path);
	media_extract_path = props_get_string(PROP_MEDIA_EXTRACT_PATH,realTimeUpdate);
	
	// Definição do player de mídia
	free(media_player_regex);
	media_player_regex = props_get_string(PROP_MEDIA_PLAYER_REGEX,realTimeUpdate);
	
	return true;
	
}

/** Recarrega as propriedades dinâmicas do arquivo */
boolean dep_loader_refresh(void) {
	
	return dep_loader_init_dynamic(true);
	
}

/** Inicializa os ponteiros globais com os caminhos absolutos */
boolean dep_loader_init(void) {
	
	com_screen_clear();
	
	sup_media_init();
	auto_del_init ();
	
	// Diretório raiz do programa
	main_path = props_get_string(PROP_MAIN_PATH,false);
	
	// Diretório de mídia criptografada
	media_storage_path = file_utils_mount_path(2,main_path,DEP_STORAGE_PATH);
	
	// Arquivo de Senha
	password_file_path = file_utils_mount_path(2,main_path,DEP_PASSWORD_FILENAME);

	// Arquivo de Banco de Dados
	database_main_path = file_utils_mount_path(2,main_path,DEP_DB_FILENAME);

	// Arquivo Temporário de Banco de Dados
	database_temp_path = file_utils_mount_path(2,main_path,DEP_DB_TMP_FILENAME);
	
	// Arquivo de Backup do Banco de Dados
	database_bkps_path  = file_utils_mount_path(2,main_path,DEP_DB_BKP_FILENAME);
	
	// Comando do gerenciador de arquivos
	file_manager_regex  = props_get_string(PROP_FILE_MANAGER_REGEX,false);
	
	dep_loader_init_dynamic(false);
	dep_loader_list();
	
	return true;
}

/** Desaloca as estruturas de dados da memória principal */
boolean dep_loader_free(void) {
	
	com_free_all(4,main_path,media_storage_path,media_extract_path,media_player_regex);
	com_free_all(3,database_main_path,database_temp_path,database_bkps_path);
	com_free_all(2,password_file_path,file_manager_regex);
	
	sup_media_free();
	auto_del_free ();
	
	return true;
	
}
