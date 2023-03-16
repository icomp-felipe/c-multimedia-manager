#include "commons.h"
#include "propertiesManager.h"
#include "fileUtils.h"
#include "stringUtils.h"

/** Estrutura de dados que armazena a lista de arquivos suportados */
nodeStringList sup_media_raw_list;

/** Imprime a lista de arquivos suportados */
boolean sup_media_list(void) {
	
	printf("* Lista de Arquivos Suportados     : ");
	str_utils_list_print(sup_media_raw_list);
	
	return true;
}

boolean sup_media_init_dynamic(boolean realTimeParse) {
	
	sup_media_raw_list = props_get_string_list(PROP_MEDIA_SUPPORTED_LIST,realTimeParse);
	
	return true;
}

/** Inicializa a estrutura de dados que armazena
 *  os formatos de mídia suportados pelo sistema. */
boolean sup_media_init(void) {
	
	return sup_media_init_dynamic(false);
}

/** Verifica se um arquivo de entrada pode ser importado para o sistema,
 *  de acordo com as extensões cadastradas no arquivo de propriedades. */
boolean sup_media_is_supported(const char* filename) {
	return com_extension_contains_in_list(filename,sup_media_raw_list);
}

/** Desaloca a lista da memória principal */
boolean sup_media_free(void) {
	
	str_utils_list_free(sup_media_raw_list);
	
	return true;
}

/** Recarrega a lista de arquivos suportados do arquivo de propriedades */
boolean sup_media_refresh(void) {
	
	sup_media_free();
	sup_media_init_dynamic(true);
	
	return true;
}
