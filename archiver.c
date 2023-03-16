#include <stdio.h>
#include <stdlib.h>
#include "libmedia.h"
#include "databaseDriver.h"
#include "fileUtils.h"
#include "logger.h"
#include "mediaConverter.h"
#include "commons.h"
#include "dependencyLoader.h"
#include "autoDelete.h"
#include "supportedMedia.h"
#include "archiver.h"
#include "fileOperations.h"

db_dir_pont arch_dir_import(db_dir_pont dir, const char* new_dir_name) {
	
	db_dir_pont  new_dir      = db_drv_dir_create (new_dir_name,dir);
	db_node_pont new_dir_node = db_drv_node_create(DB_DRV_DIR_TYPE,new_dir);
			
	db_drv_node_insert_sorted(dir,new_dir_node);
	
	return new_dir;
}

// db_file_pont media_conv_encryption_wizard2(tipoArquivo banco, const char* full_path, const char* relative_path, const char* filename_only)
void arch_file_import(tipoArquivo banco, db_dir_pont dir, const char* new_file_full_path, const char* reference, unsigned short int* status) {
	
	char* file_name_only     = file_utils_extract_filename(new_file_full_path);
	char* file_relative_path = (reference) ? file_utils_prepare_relative_path(new_file_full_path,reference) : file_name_only;
	
	// Primeiro verifico se o arquivo deve ser excluído (se sua extensão está inserida
	// na lista de extensões a excluir do arquivo program.properties) ...
	if (auto_del_is_deletable(file_name_only)) {
		//com_fprintln(stderr,"-> Arquivo '%s' excluído",file_relative_path);
		remove(new_file_full_path);
		status[ARCH_FILE_DELETED]++;
		return;
	}
	
	// ... depois verifico se o arquivo é suportado (se sua extensão está inserida
	// na lista de extensões suportadas do arquivo program.properties) ...
	if (!sup_media_is_supported(file_name_only)) {
		//com_fprintln(stderr,"-> Arquivo '%s' ignorado",file_relative_path);
		status[ARCH_FILE_IGNORED]++;
		return;
	}
	
	// ... e se o mesmo já foi inserido no sistema...
	if (arquivoJaInserido(banco,file_name_only)) {
		//com_fprintln(stderr,"-> Arquivo '%s' ignorado (já existe no sistema)",file_relative_path);
		status[ARCH_FILE_IGNORED]++;
		return;
	}
	
	// Criptografando o arquivo
	db_file_pont new_file = media_conv_encryption_wizard(banco,new_file_full_path,file_relative_path,file_name_only);
	
	if (!new_file) {
		//puts("x");
		status[ARCH_FILE_FAILED]++;
		return;
	}
	//else
		//puts("ok");
	
	db_node_pont new_file_node = db_drv_node_create(DB_DRV_FIL_TYPE,new_file);
	db_drv_node_insert_sorted(dir,new_file_node);
	
	status[ARCH_FILE_IMPORTED]++;
}

void arch_import_dir_recursive(tipoArquivo banco, db_dir_pont base, const char* path_to_import, const char* reference, unsigned short int* status) {
	
	tipoDirectory cur_dir_struct = opendir(path_to_import);
	nodeDirectory cur_dir_data;

	// Variável que armazena o caminho absoluto
	char *full_path_aux;
	
	// Se eu não conseguir abrir o diretório, paro por aqui mesmo
	if (!cur_dir_struct) {
		status[ARCH_DIR_FAILED]++;
		return;
	}
	
	// Percorrendo o diretório recursivamente e listando arquivos e pastas
	while ((cur_dir_data = readdir(cur_dir_struct)) != NULL) {

		// Aqui monto o caminho absoluto para cada entrada encontrada (e pulo os diretórios '.' e '..')
		if (!(str_utils_equals(cur_dir_data->d_name,".") || str_utils_equals(cur_dir_data->d_name,"..")))
			full_path_aux     = file_utils_mount_path(2,path_to_import,cur_dir_data->d_name);
		else
			continue;

		// Processamento de diretório: se encontro um diretório, o incluo na lista e vou varrendo seus arquivos recursivamente
		if (cur_dir_data->d_type == DT_DIR) {
				
			db_dir_pont new_dir = arch_dir_import(base,cur_dir_data->d_name);
			arch_import_dir_recursive(banco,new_dir,full_path_aux,reference,status);
			
		}
		
		// Processamento de arquivo: se encontro um arquivo, realizo a importação do mesmo para o sistema e depois insiro na lista
		else if (cur_dir_data->d_type == DT_REG)
			arch_file_import(banco,base,full_path_aux,reference,status);
		
		// Liberando recursos
		free(full_path_aux);
		
	}
	
	closedir(cur_dir_struct);
	rmdir   (path_to_import);
	
	status[ARCH_DIR_IMPORTED]++;
}

void arch_import_preloader(tipoArquivo banco, const char* path_to_import, const char* internal_path_reference, unsigned short int* status) {
	
	db_dir_pont base = db_drv_dir_create(".",NULL);
	
	if (file_utils_is_file(path_to_import))
		arch_file_import(banco,base,path_to_import,NULL,status);
		
	else if (file_utils_is_dir(path_to_import))
		arch_import_dir_recursive(banco,base,path_to_import,path_to_import,status);
		
	else {
		com_fprintln(stderr,"x O caminho '%s' não pôde ser importado!",path_to_import);
		status[ARCH_PATH_FAILED]++;
	}
	
	//db_drv_dir_print(base,db_mode_iterative);
	db_drv_dir_free (base);
}

/** Automatiza o processo de criptografia a todos os arquivos de entrada */
boolean arch_conv_import_wizard(int argc, char** argv) {
	
	srand(time(NULL));
	
	// Primeiramente, inicializo os ponteiros de arquivos e diretórios do sistema.
	dep_loader_init();
	dep_loader_self_test();
	
	// Abro o arquivo de banco de dados...
	tipoArquivo banco = abreBanco("rb");
	
	if (!banco)
		return false;
	
	short index;
	
	// Tratamento dos status de acordo com a quantidade de constantes definidas em "archiver.h"
	unsigned short int status[7];
	memset(status,0,7 * sizeof(unsigned short int));
	
	puts("=====| Importação de Arquivos |=====\n");
	
	// ... e, se tudo der certo, processo os arquivos de mídia,
	for (index=1;index<argc;index++)
		arch_import_preloader(banco,argv[index],NULL,status);
	
	puts       ("\n=====| Estatísticas |=====\n");
	com_println("* Arquivo(s) processado(s) com sucesso: %d",status[2]);
	com_println("* Arquivo(s) processado(s) com falha  : %d\n",status[3]);
	
	com_println("* Arquivo(s) ignorado(s): %d",status[1]);
	com_println("* Arquivo(s) excluído(s): %d\n",status[0]);
	
	com_println("* Diretório(s) importado(s) com sucesso: %d",status[5]);
	com_println("* Diretório(s) processado(s) com falha : %d\n",status[4]);
	
	com_println("* Erro(s) de caminho de arquivo: %d",status[6]);
	
	
	// ... e fecho o banco de dados.
	file_utils_close(banco);
	
	dep_loader_free();
	
	#ifdef WIN32
		com_pause();
	#endif
	
	return true;
}

/** Descriptografa todos os arquivos do Banco de Dados */
boolean arch_extract_all(void) {
	
	unsigned short int sucesso = 0;
	unsigned short int falhas  = 0;
	tipoIndice aux;
	
	// Aqui abro o arquivo de banco de dados...
	tipoArquivo banco = abreBanco("rb");
	
	// ... e verifico se ele é válido e se possui algum registro.
	if ((file_utils_is_not_OK(banco)) || (file_utils_size(banco) == 0)) {
		com_println("\nx Nenhum arquivo processado!");
		com_pause();
		return false;
	}
	
	com_screen_clear();
	
	com_println(":: Extraindo arquivos em: '%s'\n",dep_loader_get(DEP_MEDIA_EXTRACT_PATH));
	
	// Se tudo estiver ok,começo a brincadeira! Varrendo o bando de dados...
	while (!feof(banco)) {
		
		// ... lendo cada registro...
		fread(&aux,sizeof(tipoIndice),1,banco);
		
		// ... til the end...
		if (feof(banco))
			break;
		
		// ... descriptografando todos os arquivos para o diretório de saída de mídia.
		if (media_conv_decrypt(aux.hash,true))
			sucesso++;
		else
			falhas ++;
		
	}
	
	// Aqui faço apenas a impressão da mensagem de status...
	if (falhas == 0)
		com_println("\n:: Todos os %u arquivos foram descriptografados com sucesso!",sucesso);
	else
		com_println("\n:: Infelizmente %u arquivos não puderam ser descriptografados.",falhas);
	
	// ... e fecho o arquivo de banco de dados.
	file_utils_close(banco);
	
	com_pause();
	
	return true;
}

/** Remove a mídia criptografada do sistema */
boolean arch_media_delete(const char* hash) {
	
	const char* stor_path = dep_loader_get (DEP_MEDIA_STORAGE_PATH);
	char* full_path = file_utils_mount_path(2,stor_path,hash);
	
	remove(full_path);
	free  (full_path);
	
	return true;	
}
