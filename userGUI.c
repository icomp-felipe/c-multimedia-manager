#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "dependencyLoader.h"
#include "libmedia.h"
#include "commons.h"
#include "passwordManager.h"
#include "indexManager.h"
#include "fileUtils.h"
#include "fileOperations.h"
#include "mediaTools.h"
#include "archiver.h"

/** Mostra o menu de alteração de senha do sistema */
boolean user_gui_password_manager(void) {
	
	com_screen_clear();
	
	if (!passwd_man_parse_wizard())
		passwd_man_update_key_wizard();
	
	com_pause();
	
	return true;
}

/** Exibe o menu principal */
boolean user_gui_main_dialog(void) {
	
	com_print_version();
	
	com_println("1. Para visualizar os arquivos");
	com_println("2. Para extrair todos os arquivos");
	com_println("3. Para reparar o banco de dados");
	com_println("4. Para trocar a senha do sistema");
	com_println("0. Para sair\n");
	
	printf("> ");
	
	return true;
}

boolean user_gui_dialog_01(void) {
	return media_tools_main_gui();
}

boolean user_gui_dialog_02(void) {

	arch_extract_all();
	
	return true;
}

boolean user_gui_dialog_03(void) {
	return db_drv_repair_wizard();
}

boolean user_gui_dialog_04(void) {
	
	com_screen_clear();
	passwd_man_update_key_wizard();
	com_pause();
	
	return true;
}

boolean user_gui_dialog_exit(void) {
	
	puts("\n:: Até logo!");
	
	#ifdef WIN32
		com_pause();
	#endif
	
	return true;
}

/** Menu principal do programa */
boolean user_gui_main_manager(void) {
	
	// Aqui verifico se o programa possui o arquivo de senhas...
	boolean forceLogin = dep_loader_self_test();
	
	// ... senão, crio o arquivo e entro no sistema.
	if (forceLogin)
		if (!passwd_man_parse_wizard())
			return false;
	
	int option=5;
	
	// ... e faço a exibição dos itens do menu principal
	do {
		
		com_screen_clear();
		user_gui_main_dialog();
		scanf("%d",&option);
		
		switch(option) {
			
			case FIRST:
				user_gui_dialog_01();
				break;
				
			case SECOND:
				user_gui_dialog_02();
				break;
				
			case THIRD:
				user_gui_dialog_03();
				break;
				
			case FOURTH:
				user_gui_dialog_04();
				break;
				
			case EXIT:
				user_gui_dialog_exit();
				break;
				
			default:
				com_fprintln(stderr,"\nx Opção inválida!");
				com_pause();
				break;
				
		}
	} while(option);
	
	return true;
}

/** Função principal do gerenciador de mídia */
boolean user_gui_start(void) {
	
	srand(time(NULL));
	
	dep_loader_init();
	user_gui_main_manager();
	dep_loader_free();
	
	return true;
}
