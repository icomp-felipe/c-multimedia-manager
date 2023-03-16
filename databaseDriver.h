#ifndef _DATABASE_DRIVER_H_
#define _DATABASE_DRIVER_H_

#ifndef _LIBMEDIA_H_
#include "libmedia.h"
#endif

/*********** Definição de Constantes e Estruturas de Dados ************/

// Tipos de nós
#define DB_DRV_DIR_TYPE 'D'
#define DB_DRV_FIL_TYPE 'F'

// Definição dos modos de impressão
typedef unsigned char db_print_mode;
extern const db_print_mode db_mode_recursive;
extern const db_print_mode db_mode_iterative;

extern const char* db_drv_write_mode;
extern const char* db_drv_read_mode ;

// Definição das estruturas de dados do banco de dados
typedef struct db_drv_str_node  db_node_type;
typedef struct db_drv_str_node* db_node_pont;

typedef struct db_drv_str_dir    db_dir_type;
typedef struct db_drv_str_dir*   db_dir_pont;

typedef struct db_drv_str_file  db_file_type;
typedef struct db_drv_str_file* db_file_pont;

/********** Declaração de funções com o tipo 'db_node_pont' ***********/

extern db_node_pont db_drv_node_create(unsigned char type, void* data);
extern boolean      db_drv_node_free  (db_node_pont node);
extern boolean      db_drv_node_print (db_node_pont node, db_print_mode print_mode);

/********** Declaração de funções com o tipo 'db_dir_pont' ***********/

extern db_dir_pont  db_drv_dir_create     (const char* name, db_dir_pont previous);
extern boolean      db_drv_dir_free       (db_dir_pont dir);
extern boolean      db_drv_dir_print      (db_dir_pont dir, db_print_mode print_mode);

extern char*        db_drv_get_dir_name   (db_dir_pont dir);

extern db_node_pont db_drv_dir_get_node_at(db_dir_pont dir, unsigned int position);


/********** Declaração de funções com o tipo 'db_file_pont' ***********/

extern db_file_pont db_drv_file_create(const char* file_name, const char* file_hash);
extern boolean      db_drv_file_free  (db_file_pont file);
extern boolean      db_drv_file_print (db_file_pont file);

extern char* db_drv_get_file_name(db_file_pont file);
extern char* db_drv_get_file_hash(db_file_pont file);

/********* Operações de Manipulação de Diretórios e Arquivos **********/

extern boolean db_drv_file_insert_sorted(db_dir_pont dir, db_node_pont node);
extern boolean db_drv_dir_insert_sorted (db_dir_pont dir, db_node_pont node);
extern boolean db_drv_node_insert_sorted(db_dir_pont dir, db_node_pont node);

extern void db_drv_serialize_test(void);
extern boolean db_drv_repair_wizard(void);
extern char* media_conv_get_original_filename(tipoArquivo file_source);

#endif
