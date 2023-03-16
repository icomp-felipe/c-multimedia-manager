#include "commons.h"
#include "propertiesManager.h"

/** Estrutura de dados que armazena a lista de arquivos auto-deletáveis */
nodeStringList auto_del_raw_list;

/** Imprime a lista de arquivos auto-deletáveis */
boolean auto_del_list(void) {
	
	printf("* Lista de Arquivos Auto-deletáveis: ");
	str_utils_list_print(auto_del_raw_list);
	
	return true;
}

boolean auto_del_init_dynamic(boolean realTimeParse) {
	
	auto_del_raw_list = props_get_string_list(PROP_MEDIA_AUTO_DEL_LIST,realTimeParse);
	
	return true;
}

/** Inicializa a estrutura de dados que armazena
 *  os formatos de mídia auto-deletáveis pelo sistema. */
boolean auto_del_init(void) {

	return auto_del_init_dynamic(false);
}

/** Verifica se um arquivo de entrada pode ser excluído durante o processo de
 *  importação, de acordo com as extensões cadastradas no arquivo de propriedades. */
boolean auto_del_is_deletable(const char* filename) {
	return com_extension_contains_in_list(filename,auto_del_raw_list);
}

/** Desaloca a lista da memória principal */
boolean auto_del_free(void) {
	
	str_utils_list_free(auto_del_raw_list);
	
	return true;
}

/** Recarrega a lista de arquivos auto-deletáveis do arquivo de propriedades */
boolean auto_del_refresh(void) {
	
	auto_del_free();
	auto_del_init_dynamic(true);
	
	return true;
}
