/**************************************************************
 *                Gerenciador Multimídia v.3.5
 * 
 * Escrito por: Felipe André <felipeandresouza@hotmail.com>
 * 
 * Biblioteca : fileOperations.h  Última Edição: 23/05/2016
 * 
 **************************************************************
 * 
 * Esta biblioteca concentra grande parte das operações com ar-
 * quivo, tais como criação e validação de diretórios e arqui-
 * vos, teste de integridade de mídia criptografada, descompres-
 * são, remoção e outras utilidades.
 * 
 * ************************************************************/

#ifndef _FILE_OPERATIONS_H_
#define _FILE_OPERATIONS_H_

#ifndef _LIBMEDIA_H_
#include "libmedia.h"
#endif

boolean arquivoJaInserido(tipoArquivo banco, const char* filename);
boolean removeMediaDatabase(const int index);

#endif
