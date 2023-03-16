#include "libmedia.h"
#include "stringUtils.h"

#include "commons.h"

/** Estrutura de dados que implementa um array de strings */
struct stringList {
	size_t list_size;
	char** list_data;
};

/** Cria uma nova lista de string */
nodeStringList str_utils_list_create(size_t size, char** data) {
	
	nodeStringList list = (nodeStringList) malloc(sizeof(tipoStringList));
	
	if (!list) {
		perror(":: Falha ao criar lista de string!\n");
		return NULL;
	}
	
	list->list_size = size;
	list->list_data = data;
	
	return list;
	
}

/** Imprime a lista de arquivos suportados */
boolean str_utils_list_print(nodeStringList list) {
	
	size_t i;
	
	fputc('[',stdout);
	for (i=0;i<list->list_size-1;i++)
		printf("%s,",list->list_data[i]);
	printf("%s]\n",list->list_data[i]);
	
	return true;
}

/** Desaloca a 'list' da memória principal */
boolean str_utils_list_free(nodeStringList list) {
	
	size_t* i = &list->list_size;
	
	for (;*i != 0;(*i)--)
		free(list->list_data[*i-1]);
	
	free(list);
	
	return true;
}

/** Verifica se a 'string' informada consta em 'list'.
 *  Obs.: Quando 'caseSensitive' está em 'false', todas as strings são
 *  convertidas localmente para lowerCase antes de serem comparadas. */
boolean str_utils_list_contains(const char* string, nodeStringList list, boolean caseSensitive) {
	
	unsigned short int i;
	boolean status = false;
	
	// Aqui faço uma cópia da string de origem e converto-a
	// para caixa baixa se 'caseSensitive' for 'false'
	char* string_source = strdup(string);
	if (!caseSensitive)
		str_utils_to_lower(string_source);
	
	// Vou varrendo a lista em busca de uma string igual a 'string_source'
	for (i=0; (status == false) && (i<list->list_size); i++) {
		
		// Aqui faço uma cópia da string da lista e converto-a
		// para caixa baixa se 'caseSensitive' for 'false'
		char* string_to_compare = strdup(list->list_data[i]);
		if (!caseSensitive)
			str_utils_to_lower(string_to_compare);
		
		// Por fim, verifico se elas são iguais
		if (str_utils_equals(string_source,string_to_compare))
			status = true;
		
		// E desaloco os recursos utilizados
		free(string_to_compare);
	}
	
	free(string_source);
	
	return status;
	
}

/** Verifica se duas strings são iguais */
boolean str_utils_equals(const char* string, const char* another_string) {
	return strcmp(string,another_string) == 0;
}

/** Verifica se uma string está em formato ASCII */
boolean str_utils_is_ascii(char* string) {
	
	unsigned short int i, tamanho = strlen(string);
	
	for (i=0; i<tamanho; i++)
		if (!isascii(string[i]))
			return false;
			
	return true;
}

/** Verifica se uma string é vazia */
boolean str_utils_is_empty(const char* string) {
	return str_utils_equals(string,"");
}

/** Converte todos os caracteres da 'string' para caixa baixa */
char* str_utils_to_lower(char* string) {
	
	unsigned short int i;
	
	for (i=0;i<strlen(string);i++)
		string[i] = tolower(string[i]);
	
	return string;
}

/** Inverte uma string, no caso do Windows, apenas redireciono a função */
char* str_utils_reverse(char* string) {
	
	#ifdef WIN32
		strrev(string);
		return;
	#endif
	
	int inicio = 0;
	int aux, fim;
	
	fim = strlen(string) - 1;
	
	while(inicio < fim) {
		aux = string[inicio];
		string[inicio++] = string[fim];
		string[fim--] = aux;
	}
	
	return string;
}

/** Remove os espaços em branco antes e após a string.
 *  Se a string for nula, retorno uma string vazia */
char* str_utils_trim(const char* string) {
	
	char* first = (char*) string;
	
	if (!first)
		return strdup("");
	
	// Busca o primeiro caracter que não seja espaço
	while (isspace((unsigned char) * first))
		first++;
	
	// Tratamento para o caso de a string inteira ser espaço
	if (*first == 0)
		return strdup("");
	
	char* last  = (char*) string + strlen(string) - 1;
	
	// Busca o último caracter que não seja espaço
	while (last > first && isspace((unsigned char) * last))
		last--;
	
	// Aqui calculo o tamanho da nova string (sem os espaços)
	size_t size = last - first + 2;
	
	// ... aloco a nova string na memória...
	char* without_spaces = (char*) malloc(size);
	if (!without_spaces) {
		perror(":: Falha ao alocar string!\n");
		return NULL;
	}

	// ... copio os dados...
	char* aux = without_spaces;
	while (first <= last)
		*(aux++) = *(first++);
	*aux = '\0';
	
	return without_spaces;
}

/** Imprime um diálogo de debug da 'string' estruturado da seguinte forma:
 *  1. Impressão da 'string' normalmente com a função 'printf';
 *  2. Impressão da variável 'charsToPrint';
 *  3. Se for encontrado um caracter de fim de linha '\0', a função informa
 *     ao usuário e imprime até esta posição, independentemente do valor de
 *     'charsToPrint';
 *  4. Impressão da string, caracter por caracter, separados por '|':
 *     4.1. Na primeira cadeia é impressa a string;
 *     4.2. Na segunda cadeia são impressos os índices de cada caracter, po-
 *          dendo começar a contagem a partir do '0' ou '1' de acordo com a
 *          variável 'startCounterFromZero'
 */
boolean str_utils_debug(const char* string, unsigned int charsToPrint, boolean startCounterFromZero) {
	
	unsigned int i, units, print_scores = 0;
	boolean   start, is_null_terminated = 0;
	
	// Apresentação :)
	printf("===| String Debugger |===\n\n");
	
	// Se não tiver nada pra imprimir, a função pára por aqui mesmo
	if (charsToPrint == 0) {
		puts("* There's nothing to print!");
		return true;
	}
	
	// Definindo o início do contador de caracteres
	start = (startCounterFromZero == false);
	
	/********************* Imprimindo o diálogo **********************/
	
	// Impressão do diálogo
	printf("* String received: '%s'\n",string);
	printf("* Chars to print : %u\n",charsToPrint);
	printf("* Starting counter from %s\n", startCounterFromZero ? "zero" : "one");
	
	// Se for encontrado o caracter '\0', defino um novo ponto de parada e informo ao usuário
	for (i=0; i<charsToPrint; i++) {
		if (string[i] == '\0') {
			is_null_terminated = 1;   charsToPrint = i+1;   print_scores++;
			printf("* Found end-line character at position %u!\n",charsToPrint);
			break;
		}
	}
	
	// Definindo a quantidade de 'traços' a imprimir
	print_scores += ((charsToPrint * 2) + 1);
	
	/********************* Imprimindo a string ***********************/
	
	// Imprimindo 'traços'
	for (i=0; i<print_scores; i++)
		putchar('_');
	puts("");
	
	// Imprimindo a 'string' caracter por caracter até atingir 'charsToPrint'
	// elementos ou até encontrar o caracter '\0', sempre delimitando com '|'
	for (i=0; i<charsToPrint; i++) {
		
		if (string[i] == '\0')
			break;
		else
			printf("|%c",string[i]);
		
	}
	
	// Fechando a impressão da cadeia de caracteres
	(is_null_terminated) ? puts("|\\0|") : puts("|");
	
	// Imprimindo 'traços'
	for (i=0; i<print_scores; i++)
		printf("–");
	puts ("");
	
	/********************* Imprimindo os índices *********************/
	
	// Se 'charsToPrint' >= 10, faço a impressão da cadeia de dezenas
	if (charsToPrint >= 10) {
		
		for (units=0,i=start; i<(charsToPrint+start); i++) {
			printf("|%hu",units);
			if (((i+1) % 10) == 0) units++;
		}
		
		// Fechando a impressão da cadeia de dezenas
		(is_null_terminated) ? puts("\\|") : puts("|");
	}
	
	// Impressão da cadeia de unidades
	for (units=start,i=0; i<charsToPrint; i++,units++) {
		if (units == 10) units = 0;
		printf("|%hu",units);
	}
	
	// Fechando a impressão da cadeia de unidades
	(is_null_terminated) ? puts("\\|") : puts("|");
	
	// Fechando a impressão do diálogo
	for (i=0; i<print_scores; i++)
		printf("‾");
	puts("");
	
	return true;
}

/** Gera uma string aleatória de tamanho 'size' a partir dos caracteres contidos em 'charset' */
char* str_utils_get_random_string(size_t size, const char* charset) {
	
	if (size == 0)
		return NULL;
	
	char* random_string = (char*) malloc(size + 1L);
	if  (!random_string) {
		perror(":: Falha ao alocar string!\n");
		return NULL;
	}
	
	unsigned int key, index, base = (unsigned int) (strlen(charset)-1);
	
	for (index=0; index < size; index++) {
		
		key = rand() % base;
		
		// Pequena melhoria que garante um espalhamento melhor da string
		if ((index > 0) && (random_string[index-1] == random_string[index]))
			key = rand() % base;
		
		random_string[index] = charset[key];
	}
	
	random_string[size] = '\0';
		
	return random_string;
}

/** Implementação da função 'strcmp' com suporte a caracteres especiais */
int str_utils_compare(const char* string01, const char* string02) {
	
    const signed char* aux1 = (const signed char*) (string01);
    const signed char* aux2 = (const signed char*) (string02);

    while ((*aux1 == *aux2) && *aux1) {
        aux1++;
        aux2++;
    }
    
    return (*aux1 - *aux2);
}

/** Verifica se uma 'substring' está contida em uma 'string'.
 *  Caso a 'substring' seja nula, retorno nulo. */
const char* str_utils_contains(const char* string, const char* substring) {
	return (!substring) ? NULL : strstr(string,substring);
}

/** Verifica se uma string é alfabética */
boolean str_utils_is_alpha(const char* string) {
	
	for (;*string != '\0';string++)
		if (!isalpha(*string))
			return false;
	
	return true;
}
