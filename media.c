/**************************************************************
 *                Gerenciador Multimídia v.3.5
 * 
 * Escrito por: Felipe André <felipeandresouza@hotmail.com>
 * 
 * Programa   : media.c             Última Edição: 23/05/2016
 * 
 **************************************************************
 * 
 * Programa principal (main) do Gerenciador Multimídia v.4.0
 * 
 * ************************************************************/

#include "media.h"
#include "databaseDriver.h"

/** Função principal */
int main(int argc, char** argv) {
	
	com_enable_special_characters();
	
	switch (argc) {
		
		case 1:
			user_gui_start();
			break;
			
		default:
			arch_conv_import_wizard(argc,argv);
			break;
			
	}
	
	//db_drv_serialize_test();
	
	return EXIT_SUCCESS;
	
}
