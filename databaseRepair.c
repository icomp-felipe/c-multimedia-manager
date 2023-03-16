/**************************************************************
 *                Gerenciador Multimídia v.3.5
 * 
 * Escrito por: Felipe André <felipeandresouza@hotmail.com>
 * 
 * Biblioteca : databaseRepair.c  Última Edição: 23/05/2016
 * 
 **************************************************************
 * 
 * Esta biblioteca é responsável pelas operações de reparo da
 * base de dados.
 * 
 * ************************************************************/

#include "commons.h"
#include "libmedia.h"
#include "indexManager.h"
#include "fileOperations.h"
#include "fileUtils.h"

/** Verifica se a extensão do arquivo é válida, com
 *  base na extensão do arquivo criptografado (.crm) */
short parseExt(const char* filename) {
	char* node = strstr(filename,FILE_EXT);
	
	if (node == NULL)
		return notValid;
	
	return isValid;
}

/** Prepara os dados a serem escritos no Banco de Dados */
nodeIndice preparaIndice(const char* hash) {
	
	// Aqui faço a abertura do arquivo criptografado e...
	char* absolutePath = file_utils_mount_path(2,mediaPath,hash);
	tipoArquivo media = fopen(absolutePath,"rb");
	
	// ... se eu consegui abri-lo e...
	if (media == NULL) {
		com_println("x Falha ao abrir o arquivo \"%s\"",hash);
		free(absolutePath);
		return NULL;
	}
	
	// ... o mesmo passou na verificação de integridade...
	if (!integrityCheck(media)) {
		com_println("x O arquivo %s não passou na verificação de integridade!",hash);
		file_utils_close(media);	free(absolutePath);
		return NULL;
	}
	
	nodeIndice novo;
	char filename[STRMAX];
	
	// ... extraio deste o nome original do arquivo de mídia...
	fread(filename,STRMAX,UNIT,media);
	
	// ... e preparo o índice para o banco de dados.
	novo = alocaIndice(hash,filename);
	
	file_utils_close(media);
	free(absolutePath);
	
	return novo;
}

/** Cadastra cada entrada no Banco de Dados */
short cadastraIndiceArq(tipoArquivo banco, nodeIndice node) {
	com_println("-> Processando arquivo \"%s\" como \"%s\"",node->hash,node->filename);
	return fwrite(node,sizeof(tipoIndice),UNIT,banco);
}

/** Recadastra cada arquivo no Banco de Dados */
short recadastra(tipoArquivo banco, const char* index) {
	
	// Aqui obtenho o índice do arquivo...
	nodeIndice novo = preparaIndice(index);
	
	if (novo == NULL)
		return EXIT_FAILURE;
	
	// ... e o cadastro na base de dados.
	cadastraIndiceArq(banco,novo);
	
	free(novo);
	
	return EXIT_SUCCESS;
}

// @Deprecated
/** Conta quantos arquivos .crm há no diretório informado */
int contaArquivos(const char* path) {
	
    tipoDirectory directory;
    nodeDirectory entrada;
 
	// Apenas abro o diretório de mídia e,
    directory = opendir(path);
 
	// ... se tudo der certo...
    if (!directory) {
		com_println("x Não foi possivel abrir o diretório \"%s\"!",path);
        return EXIT_FAILURE;
    }
    
	int counter=0;
	
	// ... conto quantos arquivos de mídia válidos este possui...
    while ((entrada = readdir(directory)))
       if (parseExt(entrada->d_name))
			counter++;

	// ... e desaloco os recursos utilizados.
	free(entrada);
	closedir(directory);
	
	return counter;
}

/** Analisa os arquivos do diretório e os recadastra no Banco de Dados */
short parseDirectory(tipoArquivo banco, const char* path) {

    tipoDirectory directory;
    nodeDirectory entrada;
 
	// Aqui faço a abertura do diretório de mídia e,
    directory = opendir(path);
    
    // ... se este foi aberto com sucesso e,
    if (!directory) {
		com_println("\nx Não foi possivel abrir o diretório \"%s\"!",path);
        return 0;
    }
    
    // ... há arquivos a serem processados,
    if (contaArquivos(path) == 0)
		return 0;
	
    short counter = 0;
    
    // ... logo, vou recadastrando todos os arquivos válidos do diretório na base de dados.
    while ((entrada = readdir(directory))) {
		
		if (parseExt(entrada->d_name)) {
			recadastra(banco,entrada->d_name);
			counter++;
		}
		
	}
	
	// ... e desaloco os recursos utilizados.
    free(entrada);
	closedir(directory);
	printf("\n");
	
    return counter;
}

// @Deprecated
/** Repara o arquivo de Banco de Dados */
short reparar(void) {
	
	com_screen_clear();
	com_println("* Reparando a base de dados...\n");
	
	// Primeiramente crio uma cópia do arquivo de banco de dados antes de ...
	rename(databasePath,databaseBkpPath);
	remove(databasePath);
	
	// ... criar um novo.
	tipoArquivo banco = abreBanco("wb");
	
	if (file_utils_is_not_OK(banco))
		return EXIT_FAILURE;
	
	// Se tudo estiver ok, faço a análise e processamento de todos
	// os arquivos do diretório de mídia criptografada.
	short arquivosProcessados = parseDirectory(banco,mediaPath);
	
	// Aqui faço a impressão de algumas informações
	if (arquivosProcessados == 0)
		com_println("x Nenhum arquivo processado!");
	else if (arquivosProcessados == 1)
		com_println("* 1 arquivo foi processado com sucesso!");
	else
		com_println("* %d arquivos foram processados com sucesso!",arquivosProcessados);
	
	file_utils_close(banco);
	com_println("\n* Base de dados sincronizada com o(s) arquivo(s) de mídia.");
	com_println("* Backup da base de dados salva em \"%s\"",databaseBkpPath);
	
	com_pause();
	
	return EXIT_SUCCESS;
}
