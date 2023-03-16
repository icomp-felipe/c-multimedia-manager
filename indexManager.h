/**************************************************************
 *                Gerenciador Multimídia v.3.5
 * 
 * Escrito por: Felipe André <felipeandresouza@hotmail.com>
 * 
 * Biblioteca : indexManager.h    Última Edição: 23/05/2016
 * 
 **************************************************************
 * 
 * Esta biblioteca encapsula a maioria das operações com índi-
 * ces de mídia criptografada.
 * 
 * ************************************************************/

#ifndef _INDEX_MANAGER_H_
#define _INDEX_MANAGER_H_

#ifndef _LIBMEDIA_H_
#include "libmedia.h"
#endif

int contaIndices(tipoArquivo banco);

nodeIndice buscaIndice(tipoArquivo arquivo, int posicao);
nodeIndice alocaIndice(const char* hash, const char* filename);

short reparar(void);
short listarIndices();
short cadastraIndice(nodeIndice novo);
short verificaIndice(const char* filename);

#endif
