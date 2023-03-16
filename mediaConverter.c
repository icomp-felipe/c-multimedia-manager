#include "libmedia.h"
#include "fileUtils.h"
#include "commons.h"
#include "supportedMedia.h"
#include "dependencyLoader.h"
#include "autoDelete.h"
#include "fileOperations.h"
#include "indexManager.h"
#include "mediaConverter.h"
#include "databaseDriver.h"

/** Definição do tipo de mídia criptografada do sistema */
#define MEDIA_CONV_MAGIC_NUMBER "!FLP#"
#define MEDIA_CONV_MAGIC_NBSIZE 5

/** Parâmetros do algoritmo de hash */
#define RANDOM_STR_LENGTH  40L
#define RANDOM_STR_CHARSET "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"

/** Controle de versionamento de arquivo de mídia criptografada */
#define MEDIA_CONV_FILE_VER 4

/** Gera um índice de arquivo */
char* media_conv_generate_hash(void) {
	
	char* hashed_filename = str_utils_get_random_string(RANDOM_STR_LENGTH,RANDOM_STR_CHARSET);
	
	// Se já houver um arquivo com o mesmo nome criado acima, este algoritmo é chamado novamente de forma recursiva
	if (verificaIndice(hashed_filename) == EQUALS)
		return media_conv_generate_hash();

	return hashed_filename;
}

/** Criptografa o arquivo de entrada */
db_file_pont media_conv_encrypt(const char* source_full_path, const char* source_relative_path, const char* target_hashed_filename) {
	
	const char* media_storage_path = dep_loader_get(DEP_MEDIA_STORAGE_PATH);
	char* file_target_path = file_utils_mount_path(2,media_storage_path,target_hashed_filename);
	
	// Depois de declarados os arquivos de origem e destino...
	tipoArquivo file_source = fopen(source_full_path,"rb");
	tipoArquivo file_target = fopen(file_target_path,"wb");
	
	// ... verifico se eles foram abertos com sucesso...
	if (file_utils_isn_not_OK(2,file_source,file_target)) {
		free(file_target_path);
		return false;
	}
	
	size_t file_source_size = file_utils_size(file_source);
	
	char* relative_path_aux = (char*) calloc(MEDIA_CONV_PATH_SIZE+1,sizeof(char));
	strncpy(relative_path_aux,source_relative_path,MEDIA_CONV_PATH_SIZE);
	
	// Primeira escrita no arquivo de saída (Magic Number de 5 bytes)	
	fprintf(file_target,"%s",MEDIA_CONV_MAGIC_NUMBER);
	
	// Segunda escrita no arquivo de saída (Versão do arquivo)
	fprintf(file_target,"%d ",MEDIA_CONV_FILE_VER);
	
	// Terceira escrita no arquivo de saída (Tamanho do arquivo de entrada)
	fprintf(file_target,"%ld",file_source_size);
	
	// Quarta escrita no arquivo de saída (Arquivo de entrada)
	file_utils_copy_progress_bar(file_source,file_target,file_source_size,relative_path_aux);
	
	// Última escrita no arquivo e saída (Endereco Relativo do Arquivo de Entrada)
	fwrite(relative_path_aux,MEDIA_CONV_PATH_SIZE,1,file_target);
	db_file_pont node = db_drv_file_create(source_relative_path,target_hashed_filename);
	
	file_utils_close(file_source);
	file_utils_close(file_target);
	free(file_target_path);
	free(relative_path_aux);
	
	return node;
}

/** Criptografa cada arquivo individualmente */
db_file_pont media_conv_encryption_wizard(tipoArquivo banco, const char* full_path, const char* relative_path, const char* filename_only) {
	
	// ... criando o índice do arquivo criptogtafado...
	char* hashed_filename = media_conv_generate_hash();
	db_file_pont  file_struct = media_conv_encrypt(full_path,relative_path,hashed_filename);
	
	// ... e o criptografando.
	if (!file_struct)
		return NULL;
	
	// Se tudo der certo, cadastro o arquivo na base de dados...
	nodeIndice gambi = alocaIndice(db_drv_get_file_hash(file_struct),db_drv_get_file_name(file_struct));
	cadastraIndice(gambi);
	
	// ... removo o original...
	remove(full_path);

	// ... e desaloco os recursos utilizados.
	free(hashed_filename);
	
	free(gambi);
	
	return file_struct;
}

/** Lê o magic number do arquivo */
char* media_conv_get_magic_number(tipoArquivo file_source) {
	
	size_t size = MEDIA_CONV_MAGIC_NBSIZE;
	char* magic_number = (char*) malloc(size+1);
	
	fread(magic_number,size,1,file_source);
	magic_number[size] = '\0';
	
	return magic_number;
	
}

/** Lê a versão do arquivo */
int media_conv_get_file_version(tipoArquivo file_source) {
	
	int version;
	
	fscanf(file_source,"%d",&version);
	
	return version;
}

/** Verifica a integridade do arquivo criptografado, com
 *  base no número mágico escrito no início do arquivo. */
boolean media_conv_integrity_check(tipoArquivo file_source) {
	
	char* magic_number = media_conv_get_magic_number(file_source);
	boolean magic_stat = str_utils_equals(magic_number,MEDIA_CONV_MAGIC_NUMBER);
	
	free(magic_number);
		
	return magic_stat;
}

/** Verifica se a versão do arquivo informado é compatível pelo sistema. */
boolean media_conv_version_check(tipoArquivo file_source) {
	return (media_conv_get_file_version(file_source) == MEDIA_CONV_FILE_VER);	
}

/** Realiza a verificação do número mágico e da versão do arquivo */
boolean media_conv_valid_file(tipoArquivo file_source) {
	return media_conv_integrity_check(file_source) && media_conv_version_check(file_source);
}

/** Descriptografa o arquivo de entrada */
boolean media_conv_decrypt(const char* file_hash, boolean file_extract_all) {
	
	boolean status = true;
	
	// Primeiramente atualizo o diretório de extração de midia...
	dep_loader_refresh();
	
	// Segundamente preparo o nome do arquivos...
	const char* media_storage_path = dep_loader_get(DEP_MEDIA_STORAGE_PATH);
	char* file_source_path = file_utils_mount_path(2,media_storage_path,file_hash);
	
	// ... e tento abrir o arquivo de entrada.
	tipoArquivo file_source = fopen(file_source_path,"rb");
	tipoArquivo file_target;
	
	// Paro por aqui se ocorrer alguma falha de abertura do arquivo...
	if (file_utils_is_not_OK(file_source)) {
		
		/*if (!file_extract_all) {
			com_println("x Falha ao abrir o arquivo '%s' talvez ele não exista ou esteja inacessível!",file_hash);
			com_pause();
		}*/
		
		free(file_source_path);
		return false;
	}
	
	// ... ou o arquivo não passar na verificação de integridade...
	if (!media_conv_integrity_check(file_source)) {
		//com_println("x O arquivo '%s' não passou na verificação de integridade!",file_hash);
		free(file_source_path);
		file_utils_close(file_source); com_pause();
		return false;
	}
	
	// ... ou se a versão do arquivo não for compatível.
	if (!media_conv_version_check(file_source)) {
		//com_println("x A versão do arquivo '%s' não é suportada pelo sistema!",file_hash);
		free(file_source_path);
		file_utils_close(file_source); com_pause();
		return false;
	}
	
	// Se tudo estiver correto, faço a leitura do tamanho do arquivo...
	size_t file_size;
	fscanf(file_source,"%ld",&file_size);
	
	// E obtenho o nome original do arquivo
	char original_filename[MEDIA_CONV_PATH_SIZE];
	size_t file_pointer_begin = ftell(file_source);
	
	fseek(file_source,file_size,SEEK_CUR);
	fread(original_filename,MEDIA_CONV_PATH_SIZE,1,file_source);
	fseek(file_source,file_pointer_begin,SEEK_SET);
	
	const char* media_extract_path = dep_loader_get(DEP_MEDIA_EXTRACT_PATH);
	char* file_target_path;
	
	// ... abro o arquivo de saída...
	if (file_extract_all) {		// Caminho completo (inclusive subdiretórios)
		file_target_path = file_utils_mount_path(2,media_extract_path,original_filename);
		file_target = file_utils_touch(file_target_path);
	}
	else {						// Caminho absoluto ao raiz do programa
		file_target_path = file_utils_mount_path(2,media_extract_path,file_utils_extract_filename(original_filename));
		file_target = fopen(file_target_path,"wb");
	}
	
	if (file_utils_is_not_OK(file_target)) {
		//com_println("x Falha ao abrir o arquivo de saída '%s'",file_target_path);
		free (file_source_path);
		free (file_target_path);
		return false;
	}
	
	// ... e inicio a cópia dos dados.
	status = file_utils_copy_progress_bar(file_source,file_target,file_size,original_filename);
	
	// Depois de tudo ok, fecho os arquivos e desaloco os recursos utilizados.
	file_utils_close(file_source);
	file_utils_close(file_target);
	
	com_free_all(2,file_source_path,file_target_path);
	
	return status;
}

char* media_conv_get_original_filename(tipoArquivo file_source) {
	
	char original_filename[MEDIA_CONV_PATH_SIZE];
	
	fseek(file_source,(-1) * MEDIA_CONV_PATH_SIZE,SEEK_END);
	fread(original_filename,MEDIA_CONV_PATH_SIZE,1,file_source);
	
	return strdup(original_filename);
}

/** Faz o debugging de um arquivo criptografado, lendo seus principais
 *  atributos e mostrando os resultados coletados em um assistente. */
boolean media_conv_debugger(const char* file_hash) {
	
	// Preparando o nome do arquivos...
	const char* media_storage_path = dep_loader_get(DEP_MEDIA_STORAGE_PATH);
	char* file_source_path = file_utils_mount_path(2,media_storage_path,file_hash);
	
	// ... e tentando abrir o arquivo de entrada.
	tipoArquivo file_source = fopen(file_source_path,"rb");
	
	// Paro por aqui se ocorrer alguma falha...
	if (file_utils_is_not_OK(file_source)) {
		com_println("x Falha ao abrir o arquivo '%s' talvez ele não exista ou esteja inacessível!",file_hash);
		free(file_source_path);
		return false;
	}
	
	// Obtendo informacões
	// Lendo o magic number...
	char* magic_number = media_conv_get_magic_number(file_source);
	
	// Lendo a versão do arquivo
	int file_version = media_conv_get_file_version(file_source);
	
	// Lendo o tamanho do arquivo...
	size_t file_size;
	fscanf(file_source,"%ld",&file_size);
	char* readable_file_size = file_utils_readable_file_size(file_size);
	
	// E obtenho o nome original do arquivo
	char original_filename[MEDIA_CONV_PATH_SIZE];
	
	fseek(file_source,file_size,SEEK_CUR);
	fread(original_filename,MEDIA_CONV_PATH_SIZE,1,file_source);
	fclose(file_source);
	
	com_screen_clear();
	puts("=====| Encrypted File Debugger |=====\n");
	com_println("* Magic Number : '%s'",magic_number      );
	com_println("* File Version : %d"  ,file_version      );
	com_println("* Media Size   : %s"  ,readable_file_size);
	com_println("* Relative Path:'%s'" ,original_filename );
	com_pause();
	
	free(readable_file_size);
	free(file_source_path);
	free(magic_number);
	
	return true;
}
