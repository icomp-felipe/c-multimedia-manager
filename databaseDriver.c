#include "databaseDriver.h"
#include "commons.h"
#include "logger.h"
#include "fileUtils.h"
#include "mediaConverter.h"
#include "dependencyLoader.h"
#include <stdint.h>

/*********** Definição de Constantes e Estruturas de Dados ************/

/** Definição das constantes de modo de impressão */
const db_print_mode db_mode_recursive = 1;
const db_print_mode db_mode_iterative = 2;

/** Definição das constantes de modo de abertura do arquivo de banco de dados */
const char* db_drv_write_mode = "wb";
const char* db_drv_read_mode  = "rb";

/** Definição do tipo nó */
#define DB_DRV_NOD_TYPE 'N'

/** Estrutura de dados padrão pra serialização e deserialização do banco de dados */
typedef struct dv_drv_str_serial  db_ser_type;
typedef struct dv_drv_str_serial* db_ser_pont;

/** Estrutura de Dados de um nó da lista encadeada.
 * 'node_type' pode ser um diretório (DB_DRV_DIR_TYPE) ou um arquivo (DB_DRV_FIL_TYPE);
 * 'node_data' é o ponteiro para o nó propriamente dito que pode ser um diretório (db_drv_str_dir) ou arquivo (db_drv_str_file);
 * 'node_prox' é o ponteiro para o próximo nó. */
struct db_drv_str_node {
	
	uint8_t node_type;
	
	union {
		db_file_pont node_file;
		db_dir_pont  node_dir;
		uint16_t     node_data_ser_addr;
	};
	
	union {
		db_node_pont  node_prox;
		uint16_t      node_prox_ser_addr;
	};
	
};

/** Estrutura de Dados que representa um diretório.
 * 'dir_name' é o nome do diretório;
 * 'dir_node_count' representa o número de elementos contidos no diretório atual;
 * 'dir_previous' aponta para o diretório anterior. Se não houver anterior, este deve ser NULL;
 * 'dir_node_list' contém a lista encadeada de elementos (arquivos ou diretórios) */
struct db_drv_str_dir {
	
	char*        dir_name;
	uint16_t     dir_node_count;
	
	union {
		db_dir_pont dir_previous;
		uint16_t    dir_previous_ser_addr;
	};
	
	union {
		db_node_pont dir_node_list;
		uint16_t     dir_node_list_ser_addr;
	};
	
};

/** Estrutura de Dados que representa um arquivo.
 * 'file_name' deve sempre conter o caminho absoluto do arquivo;
 * 'file_hash' deve sempre conter o nome espalhado do arquivo. */
struct db_drv_str_file {
	
	char* file_name;
	char* file_hash;
	
};

/** Estrutura de dados padrão pra serialização e deserialização do banco de dados.
 *  'ser_type' é o tipo do nó
 *  'ser_relative_addr' representa o endereço relativo do nó
 *  'ser_real_addr' armazena a estrutura de dados na memória (nó,arquivo ou diretório)
 *  'ser_next' aponta para o próximo endereço da lista de nós */
struct dv_drv_str_serial {
	
	uint8_t     ser_type;
	uint16_t    ser_relative_addr;
	void*       ser_real_addr;
	db_ser_pont ser_next;
	
};

/************ Operações de I/0 com o tipo 'db_ser_pont' **************/

/** Cria um nó de serialização */
db_ser_pont db_drv_ser_create(uint8_t type, uint16_t relative_addr, void* real_addr) {
	
	db_ser_pont node = (db_ser_pont) malloc(sizeof(db_ser_type));
	
	node->ser_type = type;
	node->ser_relative_addr = relative_addr;
	node->ser_real_addr = real_addr;
	node->ser_next = NULL;
	
	return node;
}

/** Insere um nó de serialização sempre ao fim da lista de nós */
boolean db_drv_ser_insert(db_ser_pont* list, db_ser_pont node) {
	
	if (*list == NULL)
		*list = node;
	else {
		
		db_ser_pont aux = *list;
		
		while (aux->ser_next != NULL)
			aux = aux->ser_next;
		aux->ser_next = node;
		
	}
	
	return true;
}

/** Desaloca da memória todos os nós de serialização contidos na 'list' */
boolean db_drv_ser_free_list(db_ser_pont list) {
	
	db_ser_pont aux;
	
	while (!list) {
		
		aux  = list;
		list = list->ser_next;
		free(aux);
		
	}
	
	return true;
	
}

/** Imprime todos os nós da lista encadeada de nós serializáveis */
void db_drv_ser_print(db_ser_pont list) {
	
	db_dir_pont  dir;
	db_file_pont file;
	
	while (list != NULL) {
		
		printf("Node: %hu - Type: '%c'",list->ser_relative_addr,list->ser_type);
		
		switch (list->ser_type) {
			
			case DB_DRV_DIR_TYPE:
				dir = (db_dir_pont) list->ser_real_addr;
				printf(" - [Dir Name: '%s' - Dir Size: %hu]\n",dir->dir_name,dir->dir_node_count);
				break;
				
			case DB_DRV_FIL_TYPE:
				file = (db_file_pont) list->ser_real_addr;
				printf(" - [File Name: '%s' - File Hash: '%s']\n",file->file_name,file->file_hash);
				break;
			
			case DB_DRV_NOD_TYPE:
				puts("");
				break;
				
			default:
				perror("Unrecognized type!\n");
				break;
			
		}
		
		list = list->ser_next;
		
	}
	
}

/*************** Operações úteis de (de)serialização *****************/

/** Registra e recupera de 'list' um endereço relativo à estrutura de dados contida em 'pointer' */
uint16_t db_drv_get_relative_addr(db_ser_pont* list, uint8_t type, void* pointer) {
	
	// Se o ponteiro for NULL, seu endereço é zero
	if (!pointer) {
		//puts("relative addr: NULL");
		return 0;
	}
	
	// Inicializando o primeiro endereço relativo
	uint16_t relative_addr = 1;
	
	// Primeiro elemento da lista
	if (*list == NULL)
		*list = db_drv_ser_create(type,relative_addr,pointer);
		
	// Demais elementos da lista
	else {
		
		db_ser_pont aux = *list;
		
		while (aux->ser_next != NULL) {
			
			if (aux->ser_real_addr == pointer)
				return aux->ser_relative_addr;
			
			relative_addr++;
			aux = aux->ser_next;
			
		}
		
		// Tratamento do último item
		if (aux->ser_real_addr == pointer)
			relative_addr = aux->ser_relative_addr;
		else
			aux->ser_next = db_drv_ser_create(type,++relative_addr,pointer);
			
	}
	
	return relative_addr;
}

/** Recupera de 'list' o endereço real de uma estrutura de dados identificada por 'relative_addr' */
void* db_drv_get_real_addr(uint16_t relative_addr, db_ser_pont list) {
	
	if (relative_addr == 0)
		return NULL;
		
	while (list != NULL) {
		
		if (list->ser_relative_addr == relative_addr)
			return list->ser_real_addr;
			
		list = list->ser_next;
		
	}
	
	return NULL;
	
}

/** Função de deserialização de uma string */
char* db_drv_string_deserialize(size_t length, tipoArquivo file) {
	
	char* string = (char*) malloc(length+1);
	
	fread(string,length,1,file);
	string[length] = '\0';
	
	return string;
}

/************ Operações de I/0 com o tipo 'db_node_pont' **************/

/** Cria um nó padrão */
db_node_pont db_drv_node_create(unsigned char type, void* data) {
	
	db_node_pont node = (db_node_pont) malloc(sizeof(db_node_type));
	node->node_type = type;
	node->node_file = data;
	node->node_prox = NULL;
	
	return node;
}

db_node_pont db_drv_node_ser_create(uint8_t node_type, uint16_t node_data, uint16_t node_prox) {
	
	db_node_pont node = (db_node_pont) malloc(sizeof(db_node_type));
	node->node_type = node_type;
	node->node_data_ser_addr = node_data;
	node->node_prox_ser_addr = node_prox;
	
	return node;
}

/** Libera um nó da memória */
boolean db_drv_node_free(db_node_pont node) {
	
	switch (node->node_type) {
		
		case DB_DRV_DIR_TYPE:
			db_drv_dir_free (node->node_dir);
		break;
		
		case DB_DRV_FIL_TYPE:
			db_drv_file_free(node->node_file);
		break;
		
		default:
			free(node->node_file);
		break;
		
	}
	
	free(node);
	
	return true;
	
}

/** Imprime as informações de um nó da base de dados (recursivamente) */
boolean db_drv_node_print_recursive(db_node_pont node) {
	
	while (node) {
		
		if (node->node_type == DB_DRV_DIR_TYPE)
			db_drv_dir_print (node->node_dir,db_mode_recursive);
		else
			db_drv_file_print(node->node_file);
		puts("");
		
		node = node->node_prox;
		
	}
	
	return true;
}

/** Imprime as informações de um nó da base de dados (iterativamente) */
boolean db_drv_node_print_iterative(db_node_pont node) {
	
	while (node) {
		
		switch (node->node_type) {
			
			case DB_DRV_DIR_TYPE:
				com_println("[DIR] %s",node->node_dir->dir_name);
				break;
		
			case DB_DRV_FIL_TYPE:
				com_println("[FIL] %s",node->node_file->file_name);
				break;
			
		}
		
		node = node->node_prox;
		
	}
	
	return true;
}

/** Função de impressão de nós */
boolean db_drv_node_print(db_node_pont node, db_print_mode print_mode) {
	
	if (print_mode == db_mode_recursive)
		return db_drv_node_print_recursive(node);
	
	if (print_mode == db_mode_iterative)
		return db_drv_node_print_iterative(node);
	
	return false;
}

/** Função de serialização de um nó */
boolean db_drv_node_serialize(db_node_pont node, db_ser_pont* list, tipoArquivo db) {
	
	uint8_t  node_type = node->node_type;
	
	uint16_t node_data = db_drv_get_relative_addr(list,node_type,node->node_file);
	uint16_t node_prox = db_drv_get_relative_addr(list,DB_DRV_NOD_TYPE,node->node_prox);
	
	fwrite(&node_type,sizeof(node_type),1,db);
	fwrite(&node_data,sizeof(node_data),1,db);
	fwrite(&node_prox,sizeof(node_prox),1,db);
	
	return true;
}

/** Função de deserialização de um nó */
db_node_pont db_drv_node_deserialize(tipoArquivo db) {
	
	uint8_t  node_type;
	uint16_t node_data;
	uint16_t node_prox;
	
	fread(&node_type,sizeof(node_type),1,db);
	fread(&node_data,sizeof(node_data),1,db);
	fread(&node_prox,sizeof(node_prox),1,db);
	
	return db_drv_node_ser_create(node_type,node_data,node_prox);
}

/** Atualiza os endereços reais das dependências de um nó */
boolean db_drv_node_relink(void* pointer, db_ser_pont list) {
	
	db_node_pont node = (db_node_pont) pointer;
	
	node->node_file = db_drv_get_real_addr(node->node_data_ser_addr,list);
	node->node_prox = (db_node_pont) db_drv_get_real_addr(node->node_prox_ser_addr,list);
	
	return true;
}

/************ Operações de I/0 com o tipo 'db_dir_pont' ***************/

/** Instancia um diretório */
db_dir_pont db_drv_dir_create(const char* name, db_dir_pont previous) {
	
	db_dir_pont dir = (db_dir_pont) malloc(sizeof(db_dir_type));
	
	dir->dir_name = strdup(name);
	dir->dir_node_count = 0;
	dir->dir_previous = previous;
	dir->dir_node_list = NULL;
	
	return dir;
}

db_dir_pont db_drv_dir_ser_create(char* name, uint16_t previous, uint16_t node_count, uint16_t node_list) {
	
	db_dir_pont dir = (db_dir_pont) malloc(sizeof(db_dir_type));
	
	dir->dir_name = name;
	dir->dir_node_count = node_count;
	dir->dir_previous_ser_addr  = previous;
	dir->dir_node_list_ser_addr = node_list;
	
	return dir;
}

/** Recupera o 'position'-ésimo elemento da lista de nós de 'dir' */
db_node_pont db_drv_dir_get_node_at(db_dir_pont dir, unsigned int position) {

	// Se o índice for maior que o tamanho da lista de entradas no diretório, paro por aqui mesmo
	if (position > dir->dir_node_count) {
		perror(":: Directory.ArrayIndexOutOfBoundsException\n");
		return NULL;
	}

	db_node_pont aux = dir->dir_node_list;
	
	for (;position > 1;position--)
		aux = aux->node_prox;
	
	return aux;
}

/** Libera um diretório da memória */
boolean db_drv_dir_free(db_dir_pont dir) {
	
	db_node_pont aux;
	
	free(dir->dir_name);
	
	while (dir->dir_node_list != NULL) {
		
		aux = dir->dir_node_list;
		dir->dir_node_list = dir->dir_node_list->node_prox;
		db_drv_node_free(aux);
		
	}
	
	free(dir);
	
	return true;
	
}

/** Imprime as informaçõs de um diretório (recursivamente) */
boolean db_drv_dir_print_recursive(db_dir_pont dir) {
	
	com_println("Dir Name: %s\n",dir->dir_name);
	db_drv_node_print_recursive(dir->dir_node_list);
	
	return true;
}

/** Imprime as informaçõs de um diretório (iterativamente) */
boolean db_drv_dir_print_iterative(db_dir_pont dir) {
	
	com_println("Dir Name: %s\n",dir->dir_name);
	db_drv_node_print_iterative(dir->dir_node_list);
	
	return true;
}

/** Imprime as informaçõs de um diretório */
boolean db_drv_dir_print(db_dir_pont dir, db_print_mode print_mode) {
	
	if (print_mode == db_mode_recursive)
		return db_drv_dir_print_recursive(dir);
	
	if (print_mode == db_mode_iterative)
		return db_drv_dir_print_iterative(dir);
	
	return false;
}

/** Retorna o nome do diretório informado */
char* db_drv_get_dir_name(db_dir_pont dir) {
	return dir->dir_name;
}

/** Função de serialização de diretório */
boolean db_drv_dir_serialize(db_dir_pont dir, db_ser_pont* list, tipoArquivo db) {
	
	size_t   dir_name_len = strlen(dir->dir_name);
	char*    dir_name     = dir->dir_name;
	uint16_t dir_size     = dir->dir_node_count;
	
	uint16_t dir_previous  = db_drv_get_relative_addr(list,DB_DRV_DIR_TYPE,dir->dir_previous );
	uint16_t dir_node_list = db_drv_get_relative_addr(list,DB_DRV_NOD_TYPE,dir->dir_node_list);
	
	fwrite(&dir_name_len ,sizeof(dir_name_len) ,1,db);
	fwrite(dir_name      ,dir_name_len         ,1,db);
	fwrite(&dir_size     ,sizeof(dir_size)     ,1,db);
	fwrite(&dir_previous ,sizeof(dir_previous) ,1,db);
	fwrite(&dir_node_list,sizeof(dir_node_list),1,db);
	
	return true;
}

/** Função de deserialização de diretório */
db_dir_pont db_drv_dir_deserialize(tipoArquivo db) {
	
	size_t   dir_name_len;
	char*    dir_name;
	uint16_t dir_size;
	
	uint16_t dir_previous;
	uint16_t dir_node_list;
	
	fread(&dir_name_len,sizeof(dir_name_len),1,db);
	
	dir_name = db_drv_string_deserialize(dir_name_len,db);
	
	fread(&dir_size     ,sizeof(dir_size)     ,1,db);
	fread(&dir_previous ,sizeof(dir_previous) ,1,db);
	fread(&dir_node_list,sizeof(dir_node_list),1,db);
	
	return db_drv_dir_ser_create(dir_name,dir_previous,dir_size,dir_node_list);

}

/** Atualiza os endereços reais das dependências de um diretório */
boolean db_drv_dir_relink(void* pointer, db_ser_pont list) {
	
	db_dir_pont dir = (db_dir_pont) pointer;
	
	dir->dir_previous  = (db_dir_pont ) db_drv_get_real_addr(dir->dir_previous_ser_addr ,list);
	dir->dir_node_list = (db_node_pont) db_drv_get_real_addr(dir->dir_node_list_ser_addr,list);
	
	return true;
}

/************ Operações de I/0 com o tipo 'db_file_pont' **************/

/** Cria um nó de arquivo */
db_file_pont db_drv_file_create(const char* file_name, const char* file_hash) {
	
	db_file_pont file = (db_file_pont) malloc(sizeof(db_file_type));
	file->file_name = strdup(file_name);
	file->file_hash = strdup(file_hash);

	return file;
}

db_file_pont db_drv_file_ser_create(char* file_name, char* file_hash) {
	
	db_file_pont file = (db_file_pont) malloc(sizeof(db_file_type));
	file->file_name = file_name;
	file->file_hash = file_hash;

	return file;
}

/** Libera um nó de arquivo da memória */
boolean db_drv_file_free(db_file_pont file) {
	
	free(file->file_name);
	free(file->file_hash);
	free(file);
	
	return true;
}

/** Imprime as informações de um arquivo */
boolean db_drv_file_print(db_file_pont file) {
	
	com_println("File Name: %s",file->file_name);
	com_println("File Hash: %s",file->file_hash);
	
	return true;
}

/** Recupera o nome de arquivo de um nó de arquivo */
char* db_drv_get_file_name(db_file_pont file) {
	return file->file_name;
}

/** Recupera o hash de arquivo de um nó de arquivo */
char* db_drv_get_file_hash(db_file_pont file) {
	return file->file_hash;
}

/** Função de serialização de arquivo */
boolean db_drv_file_serialize(db_file_pont file, db_ser_pont* list, tipoArquivo db) {
	
	size_t name_len = strlen(file->file_name);
	char*  name_raw = file->file_name;
	size_t hash_len = strlen(file->file_hash);
	char*  hash_raw = file->file_hash;
	
	fwrite(&name_len,sizeof(name_len),1,db);
	fwrite( name_raw,name_len        ,1,db);
	fwrite(&hash_len,sizeof(hash_len),1,db);
	fwrite( hash_raw,hash_len        ,1,db);
	
	return true;
}

/** Função de deserialização de arquivo */
db_file_pont db_drv_file_deserialize(tipoArquivo db) {
	
	size_t name_len;
	char*  name_raw;
	size_t hash_len;
	char*  hash_raw;
	
	fread(&name_len,sizeof(name_len),1,db);
	name_raw = db_drv_string_deserialize(name_len,db);
	
	fread(&hash_len,sizeof(hash_len),1,db);
	hash_raw = db_drv_string_deserialize(hash_len,db);
	
	return db_drv_file_ser_create(name_raw,hash_raw);
}

/********* Operações de Manipulação de Diretórios e Arquivos **********/

/** Inserção ordenada de um arquivo na lista de nós. Este algoritmo dá
 *  prioridade para 'arquivos'. A inserção sempre ocorre de forma crescente. */
boolean db_drv_file_insert_sorted(db_dir_pont dir, db_node_pont node) {
	
	// Trabalho com cópia do nome original por causa da função 'str_utils_to_lower'
	// já que a base do algoritmo de ordenação é a função 'strcmp' que é case-sensitive
	char* node_file_name = strdup(db_drv_get_file_name(node->node_file));
	str_utils_to_lower(node_file_name);
	
	// Iniciação dos ponteiros 'anterior (prev)' e 'percorre lista (p)'
	db_node_pont prev, p;
	prev = p = dir->dir_node_list;
	
	/** Busca pelo ponto de inserção **/
	
	// Percorrendo a lista até o NULL
	while (p != NULL) {
		
		// Como a prioridade é o arquivo, quando encontro um diretório
		// no caminho, automaticamente paro a busca
		if (p->node_type == DB_DRV_DIR_TYPE)
			break;
		
		// Aqui obtenho e manipulo o nome de arquivo de cada item da lista
		char* p_file_name = strdup(db_drv_get_file_name(p->node_file));
		str_utils_to_lower(p_file_name);
		
		// Parte comparatória
		int compare_result = str_utils_compare(node_file_name,p_file_name);
		
		// mantém a ordenação em ordem crescente...
		if (compare_result < 0) {
			free(p_file_name);
			break;
		}
		
		// ...e ignora arquivos duplicados
		if (compare_result == 0) {
			
			log_error("databaseDriver.c","O arquivo '%s' não foi inserido no sistema, pois o mesmo já existe no diretório informado!",p_file_name);
			
			free(p_file_name);
			free(node_file_name);
			db_drv_node_free(node);
			
			return false;	
		}
		
		// Desalocando recursos e...
		free(p_file_name);
		
		// ...manipulando os ponteiros
		prev = p;
		p = p->node_prox;
		
	}
	
	/************ Inserção ************/
	
	// Atualizando referências
	node->node_prox = p;
	
	// Se o elemento a ser inserido tenha que ser o primeiro da lista...
	if (prev == p)
		dir->dir_node_list = node;	// ...atualizo a cabeça da lista, senão...
	else
		prev->node_prox = node;			// ...atualizo o nó anterior
	
	// Desalocando recursos
	free(node_file_name);
	
	return true;
	
}

/** Inserção ordenada de um diretório na lista de nós */
boolean db_drv_dir_insert_sorted(db_dir_pont dir, db_node_pont node) {
	
	// Trabalho com cópia do nome original por causa da função 'str_utils_to_lower'
	// já que a base do algoritmo de ordenação é a função 'strcmp' que é case-sensitive
	char* node_dir_name = strdup(db_drv_get_dir_name(node->node_dir));
	str_utils_to_lower(node_dir_name);
	
	// Iniciação dos ponteiros 'anterior (prev)' e 'percorre lista (p)'
	db_node_pont prev, p;
	prev = p = dir->dir_node_list;
	
	/** Busca pelo ponto de inserção **/
	
	// Percorrendo a lista até o NULL
	while (p != NULL) {
		
		// Só realizo comparações se os tipos forem compatíveis (NODE_DIR), caso contrário
		// apenas percorro a lista, ou seja, 'arquivos' são automaticamente ignorados
		if (p->node_type == DB_DRV_DIR_TYPE) {
			
			// Aqui obtenho e manipulo o nome de diretório de cada item da lista
			char* p_dir_name = strdup(db_drv_get_dir_name(p->node_dir));
			str_utils_to_lower(p_dir_name);
		
			// Parte comparatória
			int compare_result = str_utils_compare(node_dir_name,p_dir_name);
			
			// mantém a ordenação em ordem crescente...
			if (compare_result < 0) {
				free(p_dir_name);
				break;
			}
			
			// ...e ignora diretórios com mesmo nome
			if (compare_result == 0) {
				
				log_info("databaseDriver.c","Ignorando diretório '%s' pois o mesmo já existe!",p_dir_name);
				
				free(p_dir_name);
				free(node_dir_name);
				db_drv_node_free(node);
				
				return true;
			}
			
			// Desalocando recursos
			free(p_dir_name);
			
		}
		
		// Manipulando ponteiros
		prev = p;
		p = p->node_prox;
		
	}
	
	/************ Inserção ************/
	
	// Atualizando referências	
	node->node_prox = p;
	
	// Se o elemento a ser inserido tenha que ser o primeiro da lista...
	if (prev == p)
		dir->dir_node_list = node;	// ...atualizo a cabeça da lista, senão...
	else
		prev->node_prox = node;			// ...atualizo o nó anterior
	
	// Desalocando recursos
	free(node_dir_name);
	
	return true;
	
}

/** Insere um nó em um diretório (implementação de inserção ordenada) */
boolean db_drv_node_insert_sorted(db_dir_pont dir, db_node_pont node) {

	// Inserindo o primeiro elemento (aqui tanto faz o tipo do nó, já que é o primeiro)
	if (dir->dir_node_list == NULL)
		dir->dir_node_list = node;
	
	else {
		
		switch (node->node_type) {
			
			case DB_DRV_FIL_TYPE:
				db_drv_file_insert_sorted(dir,node);
				break;
				
			case DB_DRV_DIR_TYPE:
				db_drv_dir_insert_sorted(dir,node);
				break;
			
			default:
				return false;
			
		}
		
	}
	
	dir->dir_node_count++;
	
	return true;		
}

/** Procedimento recursivo de carregamento de um diretório (usado para testes) */
void db_drv_dir_import(db_dir_pont base, char* path_import) {
	
	DIR* dir = opendir(path_import);
	struct dirent* de;
	char* full_path_aux;
	
	if (!dir) {
		printf(":: Could not open the directory: %s\n",base->dir_name);
		return;
	}
	
	while ((de = readdir(dir)) != NULL) {
		
		// Aqui monto o caminho absoluto para cada entrada encontrada (e pulo os diretórios '.' e '..')
		if (!(str_utils_equals(de->d_name,".") || str_utils_equals(de->d_name,"..")))
			full_path_aux = file_utils_mount_path(2,path_import,de->d_name);
		else
			continue;

		if (de->d_type == DT_DIR) {
			
			db_dir_pont  new = db_drv_dir_create(de->d_name,base);
			db_node_pont new_node = db_drv_node_create(DB_DRV_DIR_TYPE,new);
			db_drv_node_insert_sorted(base,new_node);
			db_drv_dir_import(new,full_path_aux);
			
		}
		else if (de->d_type == DT_REG) {
			
			db_file_pont new = db_drv_file_create(de->d_name,"aqui entra a hash!");
			db_node_pont new_node = db_drv_node_create(DB_DRV_FIL_TYPE,new);
			db_drv_node_insert_sorted(base,new_node);
			
		}
	}
	
	closedir(dir);
	
	
}

/********* Operações de Serialização e Deserialização do BD ***********/

/** Escreve no arquivo do 'db' o endereço relativo de 'pointer' e o seu 'type' */
boolean db_drv_ser_register(void* pointer, uint8_t type, db_ser_pont* list, tipoArquivo db) {
	
	uint16_t relative_addr = db_drv_get_relative_addr(list,type,pointer);
	
	fwrite(&relative_addr,sizeof(relative_addr),1,db);
	fwrite(&type         ,sizeof(type)         ,1,db);
	
	return true;	
}

/** Procedimento recursivo de serialização do banco de dados */
void db_drv_serialize_recursive(db_dir_pont base, db_ser_pont* list, tipoArquivo db) {
	
	// Aqui a prioridade é para a serialização de diretório
	db_drv_ser_register (base,DB_DRV_DIR_TYPE,list,db);
	db_drv_dir_serialize(base,list,db);
	
	// Depois pego sua lista de nós...
	db_node_pont aux = base->dir_node_list;
	
	// ...e vou percorrendo até seu final...
	while (aux != NULL) {
		
		// ...serializando os nós intermediários...
		db_drv_ser_register  (aux,DB_DRV_NOD_TYPE,list,db);
		db_drv_node_serialize(aux,list,db);
		
		// ...e as estruturas de dados de acordo com seus respectivos tipos.
		switch (aux->node_type) {
			
			case DB_DRV_DIR_TYPE:
				db_drv_serialize_recursive(aux->node_dir,list,db);
				break;
				
			case DB_DRV_FIL_TYPE:
				db_drv_ser_register  (aux->node_file,DB_DRV_FIL_TYPE,list,db);
				db_drv_file_serialize(aux->node_file,list,db);
				break;
				
			default:
				perror(":: Unknown format!\n");
				break;
			
		}
		
		aux = aux->node_prox;
		
	}
	
}

/** PASSO 1 do processo de deserialização: aqui apenas leio os dados 'brutos' do arquivo 'db'
 *  e vou construindo a 'list' de acordo com os tipos de dados contidos no arquivo. Vale res-
 *  saltar que, nesta etapa, os endereços ainda são relativos, pois a estrutura é incompleta! */
void db_drv_deserialize_raw(db_ser_pont* list, tipoArquivo db) {
	
	do {
		
		uint8_t  type;
		uint16_t relative_addr;
		void*    real_addr;
		
		// Aqui faço a leitura do endereço relativo e do tipo de dado registrado no arquivo...
		fread(&relative_addr,sizeof(relative_addr),1,db);	if (feof(db)) return;
		fread(&type         ,sizeof(type)         ,1,db);
		
		// ...e então, realizo a deserialização dos dados de acordo com seu tipo
		switch (type) {
			
			case DB_DRV_DIR_TYPE:
				real_addr = db_drv_dir_deserialize(db);
				break;
			
			case DB_DRV_FIL_TYPE:
				real_addr = db_drv_file_deserialize(db);
				break;
				
			case DB_DRV_NOD_TYPE:
				real_addr = db_drv_node_deserialize(db);
				break;
				
			default:
				perror(":: Unrecognized type!\n");
				break;
			
		}
		
		// Após a recuperação dos dados do arquivo, crio um nó serializável e o insiro na lista de nós
		db_ser_pont node = db_drv_ser_create(type,relative_addr,real_addr);
		db_drv_ser_insert(list,node);
		
	} while (!feof(db));
	
}

/** PASSO 2 do processo de deserialização: atualiza os endereços reais das estruturas de dados após a deserialização.
 *  Retorna sempre o endereço do diretório raiz do programa, que é sempre o primeiro objeto a ser deserializado */
db_dir_pont db_drv_linker(db_ser_pont list) {
	
	// Aqui recupero o endereço do diretório raiz do programa (primeiro dado escrito no BD).
	db_dir_pont base = (db_dir_pont) list->ser_real_addr;
	db_ser_pont aux  = list;
	
	// Depois vou percorrendo a lista e atualizando os endereços de memória das estruturas de dados
	while (aux != NULL) {
		
		switch (aux->ser_type) {
			
			case DB_DRV_DIR_TYPE:
				db_drv_dir_relink (aux->ser_real_addr,list);
				break;
				
			case DB_DRV_NOD_TYPE:
				db_drv_node_relink(aux->ser_real_addr,list);
				break;
		}
		
		aux = aux->ser_next;
		
	}
	
	return base;
}







boolean db_drv_serialize(db_dir_pont dir) {
	
	db_ser_pont list = NULL;
	tipoArquivo db = fopen("/home/felipe/Downloads/database.db","w");
	
	db_drv_serialize_recursive(dir,&list,db);
	
	fclose(db);
	
	return true;
}

db_dir_pont db_drv_deserialize(void) {
	
	db_ser_pont list = NULL;
	tipoArquivo db = fopen("/home/felipe/Downloads/database.db","rb");
	
	db_drv_deserialize_raw(&list,db);
	
	fclose(db);
	
	return db_drv_linker(list);
}

void db_drv_serialize_test(void) {
	
	db_dir_pont base = db_drv_dir_create(".",NULL);
	db_drv_dir_import(base,"/Windows");
	
	db_drv_serialize(base);
	db_drv_dir_free (base);
	
	puts("=> Deserializing...");
	db_dir_pont aux = db_drv_deserialize();
	db_drv_dir_print(aux,db_mode_recursive);
	
	db_drv_dir_free (aux);
}





/** Abre o arquivo de Banco de Dados */
tipoArquivo db_drv_database_open(const char* db_open_mode) {
	
	const char* database_path = dep_loader_get(DEP_DATABASE_MAIN_PATH);
	tipoArquivo database_file = fopen(database_path,db_open_mode);
	
	if (file_utils_is_not_OK(database_file))
		com_fprintln(stderr,"x Falha ao abrir o arquivo de banco de dados!");
		
	return database_file;
}

boolean db_drv_reparse_file(const char* file_path, const char* file_name) {
	
	tipoArquivo file = fopen(file_path,"rb");
	
	if (file_utils_is_not_OK(file)) {
		com_fprintln(stderr,"x Falha ao abrir o arquivo '%s'",file_path);
		return false;
	}
	
	if (!media_conv_valid_file(file)) {
		com_fprintln(stderr,"x O arquivo '%s' não passou na verificação de integridade!",file_path);
		return false;
	}
	
	char* original_filename = media_conv_get_original_filename(file);
	
	printf("Original filename: '%s'\n",original_filename);
	
	free(original_filename);
	
	return true;
}

/** Analisa os arquivos do diretório de mídia criptografada e os recadastra no Banco de Dados */
uint32_t db_drv_repair_parser(tipoArquivo database_file, const char* md_stor_path, uint32_t file_count) {

    tipoDirectory directory;
    nodeDirectory cur_file;
 
	// Aqui faço a abertura do diretório de mídia,
    directory = opendir(md_stor_path);
    
    // ...verifico se o diretório foi aberto com sucesso e,
    if (!directory) {
		com_fprintln(stderr,"\nx Não foi possivel abrir o diretório '%s'!",md_stor_path);
        return 0;
    }
    
    // ... se há arquivos a serem processados,
    if (file_count == 0)
		return 0;
	
	char*    media_path_aux;
    uint32_t files_succeeded = 0;
    
    // ... logo, vou recadastrando todos os arquivos válidos do diretório na base de dados.
    while ((cur_file = readdir(directory))) {
		
		if (cur_file->d_type == DT_REG) {
			
			media_path_aux  = file_utils_mount_path(2,md_stor_path,cur_file->d_name);
			
			if (db_drv_reparse_file(media_path_aux,cur_file->d_name))
				files_succeeded++;

			free(media_path_aux);
			
		}
		
	}
	
	// ... e desaloco os recursos utilizados.
	closedir(directory);
	
    return files_succeeded;
}

/** Procedimento de impressão do cabeçalho do assistente de manutenção do BD */
void db_drv_repair_header(uint32_t file_count) {
	
	com_screen_clear();
	
	puts("=====| Reparando a base de dados |=====\n");
	
	if (file_count == 0)
		puts("* Nenhum arquivo para processar!");
	else
		com_println("-> Processando %u arquivos...\n",file_count);
	
}

/** Procedimento de impressão do rodapé do assistente de manutenção do BD */
void db_drv_repair_footer(const char* db_bkps_path, uint32_t files_succeeded) {
	
	puts("");
	
	if (files_succeeded == 0)
		puts("-> Nenhum arquivo foi processado!");
		
	else if (files_succeeded == 1)
		puts("-> 1 arquivo foi processado com sucesso!");
		
	else
		com_println("-> %u arquivos foram processados com sucesso!",files_succeeded);
	
	puts("\n* Base de dados sincronizada com o(s) arquivo(s) de mídia");
	com_println("* Backup da base de dados salva em '%s'",db_bkps_path);
	
	com_pause();
	
}

/** Repara o arquivo de Banco de Dados */
boolean db_drv_repair_wizard(void) {
	
	const char* md_stor_path = dep_loader_get(DEP_MEDIA_STORAGE_PATH);
	const char* db_bkps_path = dep_loader_get(DEP_DATABASE_BKPS_PATH);
	const char* db_main_path = dep_loader_get(DEP_DATABASE_MAIN_PATH);
	
	uint32_t file_count = file_utils_dir_count(md_stor_path);
	
	// Aqui faço a impressão do cabeçalho
	db_drv_repair_header(file_count);
	
	// Primeiramente crio uma cópia do arquivo de banco de dados antes de ...
	/** rename(db_main_path,db_bkps_path); */
	
	// ... criar um novo.
	tipoArquivo database_file = NULL; /** db_drv_database_open(db_drv_write_mode); */
	
	/*if (file_utils_is_not_OK(database_file))
		return false;*/
	
	// Se tudo estiver ok, faço a análise e processamento de todos
	// os arquivos do diretório de mídia criptografada.
	uint32_t files_succeeded = db_drv_repair_parser(database_file,md_stor_path,file_count);
	
	// Aqui desaloco recursos e faço a impressão do rodapé
	file_utils_close(database_file);
	db_drv_repair_footer(db_bkps_path,files_succeeded);
	
	return true;
}
