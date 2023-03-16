/**************************************************************
 *                Gerenciador Multimídia v.3.5
 * 
 * Escrito por: Felipe André <felipeandresouza@hotmail.com>
 * 
 * Biblioteca : indexManager.c    Última Edição: 23/05/2016
 * 
 **************************************************************
 * 
 * Esta biblioteca encapsula a maioria das operações com índi-
 * ces de mídia criptografada.
 * 
 * ************************************************************/

#include "commons.h"
#include "libmedia.h"
#include "mediaTools.h"
#include "databaseRepair.h"
#include "fileOperations.h"
#include "dependencyLoader.h"
#include "propertiesManager.h"
#include "fileUtils.h"
#include "stringUtils.h"

/** Verifica se um índice já existe no Banco de Dados */
short verificaIndice(const char* filename) {
	
	tipoArquivo banco = abreBanco("rb");
	
	if (file_utils_is_not_OK(banco))
		return EXIT_FAILURE;
	
	while (!feof(banco)) {
		
		tipoIndice aux;
		fread(&aux,sizeof(tipoIndice),UNIT,banco);
		
		if (feof(banco))
			break;
			
		if (str_utils_equals(aux.hash,filename)) {
			file_utils_close(banco);
			return EQUALS;
		}
		
	}
	
	file_utils_close(banco);
	
	return EXIT_SUCCESS;
}

/** Aloca um novo índice na memória */
nodeIndice alocaIndice(const char* hash, const char* filename) {
	
	nodeIndice novo = (nodeIndice) malloc(sizeof(tipoIndice));
	
	strcpy(novo->hash,hash);
	strcpy(novo->filename,filename);
	
	return novo;
}

/** Cadastra um índice de arquivo no Banco de Dados */
short cadastraIndice(nodeIndice novo) {
	
	tipoArquivo banco = abreBanco("rb+");
	
	if (file_utils_is_not_OK(banco))
		return EXIT_FAILURE;
		
	file_utils_end_forward(banco);
	
	fwrite(novo,sizeof(tipoIndice),UNIT,banco);
	
	file_utils_close(banco);
	
	return EXIT_SUCCESS;
}

/** Lista as mídias cadastradas no sistema */
short listarIndices() {
	
	int index=1;
	tipoIndice aux;
	tipoArquivo banco = abreBanco("rb");
	
	if (!banco) return false;
	
	// Apenas percorro o arquivo de banco de dados...
	while (!feof(banco)) {
		
		// ... lendo cada índice registrado...
		fread(&aux,sizeof(tipoIndice),UNIT,banco);
		
		// ... até o final do arquivo...
		if (feof(banco))
			break;
		
		// ... e faço a impressão do índice e o nome do arquivo de mídia.
		com_println("%d. %s",index,aux.filename);
		index++;
		
	}
	
	// Como há uma stream dedicada apenas para esta função, fecho o arquivo por aqui mesmo.
	file_utils_close(banco);
	
	return true;
}

/** Conta as mídias cadastradas no sistema */
int contaIndices(tipoArquivo banco) {
	
	int index=0;
	tipoIndice aux;
	
	// Apenas percorro o arquivo de banco de dados...
	while (!feof(banco)) {
		
		// ... lendo cada índice registrado...
		fread(&aux,sizeof(tipoIndice),UNIT,banco);
		
		// ... até o final do arquivo...
		if (feof(banco))
			break;
		
		// ... e contando quantos índices este possui.	
		index++;
	}
	
	// Como há uma stream dedicada apenas para esta função, fecho o arquivo por aqui mesmo.
	file_utils_close(banco);
	
	return index;
}

/** Busca todos os dados da seleção no Banco de Dados */
nodeIndice buscaIndice(tipoArquivo arquivo, int posicao) {
	
	// Como estou trabalhando com índices, decremento uma unidade aqui.
	posicao--;
	
	// Aqui calculo a posição do índice escolhido no arquivo de banco de dados.
	size_t tamanho = file_utils_size(arquivo);
	int position = posicao * sizeof(tipoIndice);
	
	// Se o índice escolhido for válido...
	if (position >= tamanho)
		return NULL;
	
	// ... faço a leitura de todos os dados deste e retorno
    nodeIndice aux = (nodeIndice) malloc(sizeof(tipoIndice));
    
	fseek(arquivo,position,SEEK_SET);	
	fread(aux,sizeof(tipoIndice),UNIT,arquivo);
	
	return aux;
}
