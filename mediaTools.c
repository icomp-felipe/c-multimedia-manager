/**************************************************************
 *                Gerenciador Multimídia v.4.0
 * 
 * Escrito por: Felipe André <felipeandresouza@hotmail.com>
 * 
 * Biblioteca : mediaTools.c      Última Edição: 11/01/2018
 * 
 **************************************************************
 * 
 * Esta biblioteca cuida do pós-processamento dos arquivos de
 * mídia, bem como reprodução, conversão mobile e exclusão.
 * 
 * ************************************************************/

#include "commons.h"
#include "libmedia.h"
#include "indexManager.h"
#include "fileOperations.h"
#include "dependencyLoader.h"
#include "propertiesManager.h"
#include "fileUtils.h"
#include "stringUtils.h"
#include "mediaConverter.h"
#include "archiver.h"
#include <stdint.h>
#include <pthread.h>

/** Constante que define o tamanho máximo de um comando interno */
#define MEDIA_TOOLS_CMD_MAX 4096

/** Retorna um índice aleatório de arquivo */
uint16_t media_tools_get_random_index(uint16_t maxIndex) {
	return (rand() % maxIndex) + 1;
}

/** Função de Execução de Comando no Sistema Host - Implementação com pipes */
boolean media_tools_execute_command(const char* command) {
	
	char buffer[1024];
	tipoArquivo pipe = popen(command,"r");
	
	while (fgets(buffer,1024,pipe) != NULL) {}
	pclose(pipe);
	
	return true;
}

/** Reproduz a mídia de saída */
boolean media_tools_play(const char* file) {
	
	static char  command[MEDIA_TOOLS_CMD_MAX];
	const  char* media_player_regex = dep_loader_get(DEP_MEDIA_PLAYER_REGEX);
	
	dep_loader_refresh();
	
	sprintf(command,media_player_regex,file);
	com_println("-> Commandline: '%s'",command);
	
	media_tools_execute_command(command);
	
	return true;
}

/** Thread do comando de abrir diretórios */
void* media_tools_open_output_dir_thread(void* command) {
	
	media_tools_execute_command(command);
	free(command);
	
	return NULL;
}

/** Abre o diretório de saída de mídia */
boolean media_tools_open_output_dir(void) {
	
	pthread_t thread;
	char* command = (char*) malloc(MEDIA_TOOLS_CMD_MAX);
	const char* file_man_regex  = dep_loader_get(DEP_FILE_MANAGER_REGEX);
	const char* media_extr_path = dep_loader_get(DEP_MEDIA_EXTRACT_PATH);
	
	sprintf(command,file_man_regex,media_extr_path);
	com_println("-> Commandline: '%s'",command);
	
	pthread_create(&thread,NULL,media_tools_open_output_dir_thread,command);

	return true;
}

/** Função de conversão de mídia */
boolean media_tools_encode(const char* filename) {
	
 	char* command = (char*) malloc(MEDIA_TOOLS_CMD_MAX);
	
	// Aqui garanto que o comando sempre será o mais atualizado possível
	dep_loader_refresh();
	
	// Aqui monto a string do nome de arquivo a ser convertido.
	char* fileWithoutExtension = file_utils_remove_extension(filename);
	com_screen_clear();
	
	// Aqui monto a string do comando de conversão de arquivo...
	char* conv_regex = props_get_string(PROP_MEDIA_CONVERT_REGEX,true);
	sprintf(command,conv_regex,filename,fileWithoutExtension);
	
	// ... o executo, e abro o diretório de destino quando tudo estiver finalizado.
	system(command);
	puts("\n* Conversão concluída com sucesso!\n* Abrindo diretório de destino...");
	
	free(command);
	free(conv_regex);
	free(fileWithoutExtension);
	
	media_tools_open_output_dir();
	com_pause();

	return true;
}

/** Opção de exclusão permanente do arquivo de mídia */
boolean media_tools_delete(nodeIndice node, const int index) {
	
	puts  ("\n-> Você realmente deseja excluir este arquivo? (Y/n)");
	printf("\n> ");
	
	// Aqui limpo o lixo do buffer de teclado e leio a opção do usuário
	com_flush_stdin();
	char option = toupper(getchar());
	
	// Se o usuário deseja excluir o arquivo...
	if ((option == '\n') || (option == 'Y')) {
		
		// ... então o removo da base de dados e do disco.
		arch_media_delete(node->hash);
		removeMediaDatabase(index);
		
		puts("\n* Arquivo excluído com sucesso!");
		
		// Como o C puro é meio chato com o stdin, tenho que fazer manualmente os tratamentos :|
		com_pause();
		
		return true;
	}

	return false;
	
}

/** Mostra um menu de ações para o arquivo extraído */
boolean media_tools_actions_gui(tipoIndice* node, const int index) {
	
	const char* media_output_path = dep_loader_get(DEP_MEDIA_EXTRACT_PATH);
	const char* media_filename    = file_utils_extract_filename(node->filename);
	char* media_absolute_path     = file_utils_mount_path(2,media_output_path,media_filename);
	
	boolean exclusao = false;
	int option = -1;
	
	while (option && !exclusao) {
		
		com_screen_clear();
		
		// Aqui faço a impressão das informações da mídia escolhida...
		com_println("-> Item escolhido: %d. %s\n",index,node->filename);
		
		puts  ("-> O que você deseja fazer agora?\n");
		puts  ("1. Reproduzir o arquivo");
		puts  ("2. Abrir a pasta do arquivo");
		puts  ("3. Converter para celular");
		puts  ("4. Excluir permanentemente");
		puts  ("5. Visualizar informações");
		puts  ("0. Voltar ao menu anterior\n");
		printf("> ");
	
		scanf("%d",&option);
	
		switch(option) {
		
			case 1:
				media_tools_play(media_absolute_path);
				break;
			
			case 2:
				media_tools_open_output_dir();
				break;
			
			case 3:
				media_tools_encode(media_absolute_path);
				break;
				
			case 4:
				exclusao = media_tools_delete(node,index);
				break;
			
			case 5:
				media_conv_debugger(node->hash);
				break;
			
			case EXIT:
				break;
			
			default:
				com_fprintln(stderr,"\nx Opção inválida");
				break;
			
			com_pause();
		}
	}

	remove(media_absolute_path);
	free  (media_absolute_path);
	
	return true;
}

/** Descriptografa o arquivo selecionado e exibe algumas opções */
boolean media_tools_actions(nodeIndice node, int index) {

	com_screen_clear();
	
	// ...extraindo mídia...
	if (!media_conv_decrypt(node->hash,false))
		return false;
	
	// ... e exibindo algumas opções
	media_tools_actions_gui(node,index);
	
	return true;
}

/** Exibe o menu de seleção de arquivos */
boolean media_tools_main_gui(void) {
	
	uint16_t chosen, qtdArquivos;
	char option[10] = "1";
	
	nodeIndice node;
	
	com_flush_stdin();
	
	// Impressão do menu propriamente dito
	while (!str_utils_equals(option,"0")) {
		
		// Preferi trabalhar com várias streams para o banco de dados,
		// pois este é acessado por várias funções e procedimentos.
		tipoArquivo banco = abreBanco("rb");
		tipoArquivo conta = abreBanco("rb");
		
		// Basta eu testar uma dessas streams, já que o arquivo é o mesmo
		if (file_utils_isn_not_OK(2,banco,conta)) {
			return false;
		}
	
		// Verifica se o arquivo de banco de dados é "virgem"
		if ((file_utils_size(banco) == 0)) {
			puts("\nx Não há nenhum arquivo cadastrado no banco de dados!");
			file_utils_close(banco);	file_utils_close(conta);
			com_pause();
			return false;
		}
		
		qtdArquivos = contaIndices(conta);	com_screen_clear();
		
		puts("=====| Digite 0 para sair |=====\n");
		
		listarIndices();
		
		puts("*. Aleatório");
		
		com_flush_stdin();
		
		printf("\nEscolha um item aqui: ");
		scanf ("%s",option);
		
		// A opção "0" indica saída
		if (str_utils_equals(option,"0"))
			break;
		
		// A opção "*" indica que o usuário quer que o sistema escolha a mídia
		if (str_utils_equals(option,"*"))
			chosen = media_tools_get_random_index(qtdArquivos);
		else
			chosen = atoi(option);

		// Após escolhido o índice...
		node = buscaIndice(banco,chosen);
		
		// ... e este for válido...
		if (node == NULL) {
			com_perrorln("\nx Índice inválido!");
			com_pause();
		}
		
		// ... o repasso à função de opções de arquivo. E, como todo bom programador, desaloco meus recursos...
		else {
			media_tools_actions(node,chosen);
			free(node);
		}
		
		// ... e fecho todos os arquivos utilizados.
		file_utils_close(banco);
		
	}
	
	return true;
	
}
