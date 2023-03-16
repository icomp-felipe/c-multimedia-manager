/**************************************************************
 *                Gerenciador Multimídia v.3.5
 * 
 * Escrito por: Felipe André <felipeandresouza@hotmail.com>
 * 
 * Biblioteca : keyManager.c      Última Edição: 23/05/2016
 * 
 **************************************************************
 * 
 * Esta biblioteca é responsável pelas operações com o arquivo
 * de senhas do sistema.
 * 
 * ************************************************************/

#include "commons.h"
#include "libmedia.h"
#include "fileUtils.h"
#include "stringUtils.h"
#include "dependencyLoader.h"

/** Tamanho da senha */
#define PASSWD_MAX_SIZE 256

/** Palavra usada como base nos cálculos de criptografia */
static const char* passwd_man_vinegere_key = "felipe";

/** Implementação alternativa da função módulo */
int passwd_man_mod(int a, int b) {
	return (a % b + b) % b;
}

/** Implementação do algoritmo de Vigenère, usado para criptografar a senha do sistema */
char* passwd_man_cipher(const char* input, const char* key, boolean encipher) {
	
	size_t i,key_size = strlen(key);
	size_t input_size = strlen(input);
	
	// Palavra final
	char* output = (char*) malloc(input_size + 1);
	
	// Declaração de variáveis internas ao loop
	boolean char_is_upper;
	char    ciffer_offset;
	int key_index, k, nonAlphaCharCount = 0;

	// Implementação do algoritmo
	for (i = 0; i < input_size; ++i) {
		
		if (isalpha(input[i])) {
			
			char_is_upper = isupper(input[i]);
			ciffer_offset = (char_is_upper) ? 'A' : 'a';
			
			key_index = (i - nonAlphaCharCount) % key_size;
			k = (char_is_upper ? toupper(key[key_index]) : tolower(key[key_index])) - ciffer_offset;
			k = (encipher) ? k : -k;
			
			char aux = (char)((passwd_man_mod(((input[i] + k) - ciffer_offset), 26)) + ciffer_offset);
			output[i] = aux;
			
		}
		else {
			output[i] = input[i];
			nonAlphaCharCount++;
		}
		
	}

	output[input_size] = '\0';
	
	return output;
}

/** Criptografa uma palavra */
char* passwd_man_encipher(const char* input, const char* key) {	
	return passwd_man_cipher(input, key, true);
}

/** Descriptografa uma palavra */
char* passwd_man_decipher(const char* input, const char* key) {
	return passwd_man_cipher(input, key, false);
}

/** Assistente para leitura da nova senha a partir do 'stdin' */
char* passwd_man_read_key_wizard(void) {
	
	char* input01 = (char*) calloc(PASSWD_MAX_SIZE,sizeof(char));
	char* input02 = (char*) calloc(PASSWD_MAX_SIZE,sizeof(char));
	
	puts("=====| Assistente para redefinição de Senha |======\n");
	puts("=========| Regras para criação de senha |==========");
	puts("|                                                 |");
	puts("| * A senha não pode conter espaços               |");
	puts("| * A senha pode conter caracteres especiais      |");
	puts("| * A senha deve possuir no mínimo 6 caracteres   |");
	puts("| * A senha não pode ter mais que 255 caracteres  |");
	puts("===================================================");
	
	// Aqui leio a nova senha informada pelo usuário...
	printf("\n:: Digite a nova senha do sistema: ");
	scanf("%s",input01);
	
	// (verificando a validade da senha informada)
	if (strlen(input01) < 6) {
		fprintf(stderr,":: A senha informada contém menos de 6 caracteres!");
		free(input01); free(input02);
		return NULL;
	}
	
	// ... e solicito que este a confirme.
	printf(":: Confirme a senha: ");
	scanf("%s",input02);
	
	// Como estou lidando com humanos, tenho que verificar se as senhas informadas são iguais
	if (!str_utils_equals(input01,input02)) {
		fprintf(stderr,"\n:: As senhas não conferem!");
		free(input01); free(input02);
		return NULL;
	}
	
	free(input02);
	
	// Aqui retorno a nova senha (pode ser tanto 'input01' quanto
	// 'input02' já que, pra ter chegado até aqui, as senhas são as mesmas)
	return input01;
}

/** Lê uma senha a partir do teclado */
char* passwd_man_read_key(void) {
	
	static char* dialog = ":: Digite a senha: ";
	
	#ifdef WIN32
	
		char* passwd = (char*) malloc(PASSWD_MAX_SIZE * sizeof(char));
		
		printf(dialog);
		scanf("%s",passwd);
		
		return passwd;
	#else
		return strdup(getpass(dialog));
	#endif
	
}

/** Altera a senha do sistema */
boolean passwd_man_update_key_wizard(void) {
	
	// Primeiramente faço a leitura da nova senha do sistema a partir do teclado.
	char* password = passwd_man_read_key_wizard();
	
	// Se houve alguma falha, paro por aqui...
	if (!password)
		return false;
	
	// Preparando o arquivo de senhas
	const char* passwd_file_path = dep_loader_get(DEP_PASSWORD_FILE_PATH);
	tipoArquivo passwd_file      = fopen(passwd_file_path,"wb");
	
	// Se algo der errado, pego minha bike e vou embora!
	if (file_utils_is_not_OK(passwd_file)) {
		com_fprintln(stderr,":: Arquivo de senhas corrompido ou inexistente!");
		free(password);
		return false;
	}
	
	// Preparando o ambiente
	char* crypt_aux = passwd_man_encipher(password,passwd_man_vinegere_key);
	char* encrypted = (char*) malloc(PASSWD_MAX_SIZE * sizeof(char));
	
	strncpy(encrypted,crypt_aux,PASSWD_MAX_SIZE);
	
	// Escrevendo a senha no arquivo
	fwrite(encrypted,PASSWD_MAX_SIZE,1,passwd_file);
	
	// Desalocando recursos
	file_utils_close(passwd_file);
	free(password); free(encrypted); free(crypt_aux);
	
	puts("\n:: Senha do sistema alterada com sucesso!");
	
	return true;
}

/** Faz a validação da senha do sistema */
boolean passwd_man_parse_wizard(void) {
	
	boolean status = true;
	
	// Preparando o arquivo de senhas
	const char* passwd_file_path = dep_loader_get(DEP_PASSWORD_FILE_PATH);
	tipoArquivo passwd_file      = fopen(passwd_file_path,"rb");
	
	// Se algo der errado, pego minha bike e vou embora!
	if (file_utils_is_not_OK(passwd_file)) {
		com_fprintln(stderr,":: Arquivo de senhas corrompido ou inexistente!");
		return false;
	}
	
	// Primeiramente faço a leitura da senha do arquivo...
	char* file_passwd = (char*) malloc(PASSWD_MAX_SIZE * sizeof(char));
	
	fread(file_passwd,PASSWD_MAX_SIZE,1,passwd_file);
	file_utils_close(passwd_file);
	
	// ... depois, leio a senha do usuário...
	char* user_passwd = passwd_man_read_key();
	
	// ... descriptografo a senha do sistema
	char* decrypted = passwd_man_decipher(file_passwd,passwd_man_vinegere_key);
	
	// ... e verifico se ela está correta
	status = str_utils_equals(decrypted,user_passwd);

	if (!status)
		com_fprintln(stderr,"\nx Senha incorreta!");

	// Por fim, desaloco meus recursos
	free(file_passwd);
	free(user_passwd);
	
	return status;
}
