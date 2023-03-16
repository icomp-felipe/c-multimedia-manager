/**************************************************************
 *                Gerenciador Multimídia v.3.5
 * 
 * Escrito por: Felipe André <felipeandresouza@hotmail.com>
 * 
 * Biblioteca : fileOperations.c  Última Edição: 23/05/2016
 * 
 **************************************************************
 * 
 * Esta biblioteca concentra grande parte das operações com ar-
 * quivo, tais como criação e validação de diretórios e arqui-
 * vos, teste de integridade de mídia criptografada, descompres-
 * são, remoção e outras utilidades.
 * 
 * ************************************************************/

#include "commons.h"
#include "libmedia.h"
#include "indexManager.h"
#include "dependencyLoader.h"
#include "propertiesManager.h"
#include "supportedMedia.h"
#include "autoDelete.h"
#include "fileUtils.h"
#include "stringUtils.h"
#include "mediaConverter.h"
#include "archiver.h"

/** Verifica se um arquivo já foi inserido no sistema com o mesmo nome informado */
boolean arquivoJaInserido(tipoArquivo banco, const char* filename) {
	
	boolean arquivoExiste = false;
	tipoIndice aux;
	
	// Apenas percorro o arquivo de banco de dados...
	while (!feof(banco)) {
		
		// ... lendo cada índice registrado...
		fread(&aux,sizeof(tipoIndice),UNIT,banco);
		
		// ... até o final do arquivo...
		if (feof(banco))
			break;
		
		// ... e verificando se já há algum registro de "filename" neste.
		if (str_utils_equals(aux.filename,filename)) {
			arquivoExiste = true;
			break;
		}

	}
	
	// Aqui preciso "rebobinar" o arquivo, pois ele será utilizado novamente mais tarde
	rewind(banco);
	
	return arquivoExiste;
}

/** Remove um índice do banco de dados */
boolean removeMediaDatabase(const int index) {
	
	const char* db_main_path = dep_loader_get(DEP_DATABASE_MAIN_PATH);
	const char* db_temp_path = dep_loader_get(DEP_DATABASE_TEMP_PATH);
	
	tipoArquivo novoBanco  = fopen(db_temp_path,"wb");
	tipoArquivo velhoBanco = abreBanco("rb");
	
	if (file_utils_isn_not_OK(2,novoBanco,velhoBanco))
		return false;
	
	int percorre = 1;
	tipoIndice aux;
	
	// Apenas percorro o arquivo de banco de dados...
	while (!feof(velhoBanco)) {
		
		// ... lendo cada índice registrado...
		fread(&aux,sizeof(tipoIndice),UNIT,velhoBanco);
		
		// ... até o final do arquivo...
		if (feof(velhoBanco))
			break;
		
		// ... e copiando para o novo banco todos os índices, exceto o do arquivo que foi excluído.
		if (percorre != index)
			fwrite(&aux,sizeof(tipoIndice),UNIT,novoBanco);
		
		percorre++;
	}
	
	// Depois de tudo processado, fecho os arquivos...
	file_utils_close(novoBanco);
	file_utils_close(velhoBanco);
	
	// ... e os configuro.
	remove(db_main_path);
	rename(db_temp_path,db_main_path);
	
	return true;
}
