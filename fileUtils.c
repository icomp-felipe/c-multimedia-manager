#include "libmedia.h"
#include "commons.h"
#include "stringUtils.h"
#include "mediaConverter.h"
#include "logger.h"

#include <pthread.h>
#include "libprogressbar/progressbar.h"
#include "libprogressbar/statusbar.h"

#define FILE_UTILS_BUFSIZE 4096

/** Retorna 'true' se todos os 'file_count' arquivos informados foram abertos com sucesso */
boolean file_utils_isnOK(unsigned short int files_count, ...) {
	
	boolean status = true;
	va_list files;
	
	va_start(files,files_count);
	
	for (;files_count;files_count--) {
		
		tipoArquivo file = va_arg(files,tipoArquivo);
		
		if (file == NULL) {
			status = false;
			break;
		}
		
	}
	
	va_end(files);
	
	return status;
}

/** Retorna 'true' se algum dos 'file_count' arquivos informados não foi aberto com sucesso */
boolean file_utils_isn_not_OK(unsigned short int files_count, ...) {
	
	boolean status = false;
	va_list files;
	
	va_start(files,files_count);
	
	for (;files_count;files_count--) {
		
		tipoArquivo file = va_arg(files,tipoArquivo);
		
		if (file == NULL) {
			status = true;
			break;
		}
		
	}
	
	va_end(files);
	
	return status;
}

/** Retorna 'true' se o arquivo 'file' foi aberto com sucesso */
boolean file_utils_isOK(tipoArquivo file) {
	return file_utils_isnOK(1,file);
}

/** Retorna 'true' se o arquivo 'file' não foi aberto com sucesso */
boolean file_utils_is_not_OK(tipoArquivo file) {
	return file_utils_isn_not_OK(1,file);
}


/** Retorna o diretório atual */
char* file_utils_get_current_dir(void) {
	return getcwd(NULL,0);
}

/** Fecha um arquivo e adiciona o tratamento para não fechar um arquivo já fechado anteriormente */
int file_utils_close(tipoArquivo file) {
	return file_utils_is_not_OK(file) ? false : fclose(file);
}

/** Move o ponteiro de arquivo para o EOF */
int file_utils_end_forward(tipoArquivo file) {
	return fseek(file,0,SEEK_END);
}

/** Adequa os caracteres separadores de arquivo e diretório
 *  de acordo com o sistema operacional hospedeiro. */
char* file_utils_parse_path(char* path) {
	
	#if defined(WIN32) || defined(_WIN32) 
		#define char_to_replace '\\'
		#define char_to_be_replaced '/'
	#else
		#define char_to_replace '/'
		#define char_to_be_replaced '\\'
	#endif
	
	for (;*path != '\0';path++)
		if (*path == char_to_be_replaced)
			*path = char_to_replace;
	
	return path;
}

/** Monta um caminho absoluto para arquivo de acordo com 'elements' strings informadas via parâmetro.
 *  Esta função trata diversos tipos de entrada que o usuário pode fornecer, tais como excesso de
 *  barras, espaços em branco no início e fim das strings e ainda faz adaptação dos caracteres se-
 *  paradores de arquivo e diretório de acordo com o sistema operacional hospedeiro. */
char* file_utils_mount_path(size_t elements, ...) {
    
    char buffer[1024];
    
    va_list args;
    va_start(args,elements);
    
    char*  first = str_utils_trim(va_arg(args,char*));
    char*  aux;
    size_t final_length;
    
    // Copiando a primeira string dos argumentos para o buffer...
    strcpy(buffer,first);	free(first);
    final_length = strlen(buffer);
    
    // Esse é o ponteiro 'percorre' que guarda sempre o endereço do último caracter da string
    aux = &buffer[final_length-1];
    
    for (uint i=1; i<elements; i++) {
		
		// Aqui recupero um argumento da lista
		char* cur_arg = va_arg(args,char*);
		
		if (!cur_arg)
			continue;
		
		char* string  = str_utils_trim(cur_arg);
		char* pointer = string;
		
		// Se a string anterior não termina com um caracter de separação de
		// arquivo e diretório, o adiciono (no formato do linux)
		if ((*aux  != '/') && (*aux  != '\\')) {
			*(++aux) = '/';    final_length++;
		}
		
		// Se a string atual começa com um caracter de
		// separação de arquivo e diretório, o ignoro
		if ((*string == '/') || (*string == '\\'))
			string++;
		
		// Aqui copio a string atual para o buffer, ignorando o '\0'
		for (;*string != '\0';final_length++)
			*(++aux) = *(string++);
		
		free(pointer);
		
	}
    
    // Ao final do processamento, adiciono o '\0'
    *(++aux) = '\0';
    
    va_end(args);
    
    /** Depois de montar a string temporária, já vou preparando o retorno... */
    char* absolute_path = (char*) malloc(final_length + 1);
    if (!absolute_path) {
		perror(":: Falha ao alocar string!\n");
		return NULL;
	}
	
	/** ... copiando tudo para a string alocada dinamicamente... */
	strncpy(absolute_path,buffer,final_length);
	absolute_path[final_length] = '\0';
	
	/** ... e corrigindo os caracteres de separação de arquivo e pasta de acordo com o S.O. */
	file_utils_parse_path(absolute_path);

    return absolute_path;
}

/** Extrai apenas o nome do arquivo de um caminho absoluto. */
char* file_utils_extract_filename(const char* filename) {
	
	char* aux  = (char*) filename + strlen(filename) - 1;
	
	while ((aux >= filename) && (*aux != '/') && (*aux != '\\'))
		aux--;
	
	return ++aux;
}

/** Extrai apenas a extensão do arquivo de um caminho absoluto (sem o ponto). */
char* file_utils_extract_extension(const char* filename) {
	
	char* aux  = (char*) filename + strlen(filename) - 1;
	
	while ((aux >= filename) && (*aux != '.'))
		aux--;
	
	return ++aux;
}

/** Retorna o tamanho de um arquivo */
size_t file_utils_size(tipoArquivo arquivo) {

    fseek(arquivo,0,SEEK_END);
    size_t end = ftell(arquivo);
    rewind(arquivo);
	
	return end;
}

/** Carrega um arquivo de texto pra uma string */
char* file_utils_get_string_from_file(const char* path) {
	
	unsigned short i;
	tipoArquivo arquivo = fopen(path,"r");
	
	if (!arquivo) {
		printf("x Falha ao abrir o arquivo \"%s\"",path);
		return NULL;
	}
	
	size_t fileSize = file_utils_size(arquivo);
	char*  string = malloc(fileSize + 1);
	
	for (i=0;i<fileSize;i++)
		string[i] = fgetc(arquivo);
	
	string[i] = '\0';
	fclose(arquivo);
	
	return string;
	
}

/** Verifica se um arquivo ou diretório existe */
boolean file_utils_exists(const char* filename) {
	return access(filename,F_OK) != -1;
}

/** Verifica se o caminho informado é um arquivo */
boolean file_utils_is_file(const char* path) {
	
	struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

/** Verifica se o caminho informado é um diretório */
boolean file_utils_is_dir(const char* path) {
	
	struct stat path_stat;
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode);
}

/** Cria um arquivo vazio */
boolean file_utils_create_blank_file(const char* filename) {
	
	tipoArquivo arquivo = fopen(filename,"wb");
	
	if (file_utils_is_not_OK(arquivo)) {
		com_println("\nx Falha ao criar o arquivo \"%s\"...",filename);
		return false;
	}
	
	file_utils_close(arquivo);
	
	return true;
}

/** Cria um diretório */
boolean file_utils_create_directory(const char* dirname) {
	
	#ifdef WIN32
		mkdir(dirname);
	#else
		mkdir(dirname,0777);
	#endif
	
	return true;
}

/** Verifica se um arquivo existe. Caso não exista ele cria */
boolean file_utils_parse_file(const char* filename) {
	
	// Se o arquivo não existe, o crio.
	if (!file_utils_exists(filename)) {
		 file_utils_create_blank_file(filename);
		 com_println("criado");
		return false;
	}
	else
		com_println("ok");
		
	return true;
}

/** Verifica se um diretório existe. Caso não exista ele cria */
boolean file_utils_parse_dir(const char* dir_name) {
	
	if (!file_utils_exists(dir_name)) {
		 file_utils_create_directory(dir_name);
		com_println("criado");
	}
	else
		com_println("ok");
	
	return true;		
}

/** Retorna o nome do arquivo a partir do seu ponteiro */
char* file_utils_get_filename_from_pointer(tipoArquivo pointer) {
	
	#ifndef __linux__
		return NULL;
	#endif
	
	int fno, MAXSIZE = 0xFFF;
    char proclnk[MAXSIZE];    
    char* filename = (char*) malloc(MAXSIZE);
    ssize_t link;

    if (pointer != NULL) {
        fno = fileno(pointer);
        sprintf(proclnk, "/proc/self/fd/%d", fno);
        link = readlink(proclnk, filename, MAXSIZE);
        if (link < 0)
			return NULL;
        filename[link] = '\0';
    }
    
    return filename;
}

/** Copia um arquivo de uma forma mais rápida. Este cria um buffer de tamanho BUFFER
 *  e realiza a cópia de arquivos através do mesmo. Para isto, divido o tamanho total
 *  do arquivo de entrada por BUFFER, a fim de obter o número de "slots" fixos de by-
 *  tes a serem copiados de uma só vez (pelas funções fread e fwrite). Os bytes res-
 *  tantes que ficaram no último slot são copiados um a um para o arquivo de destino. */
boolean file_utils_copy(tipoArquivo file_source, tipoArquivo file_target, size_t file_source_size) {
	
	// Aqui calculo a quantidade de "slots" fixos de tamanho 'FILE_UTILS_BUFSIZE' a serem copiados
    size_t slot_counter = (file_source_size / FILE_UTILS_BUFSIZE);
    char   aux[FILE_UTILS_BUFSIZE];
    
    // Aqui começa a cópia de arquivos propriamente dita (primeiro por slots fixos...
    while (slot_counter) {
		
          fread (aux,FILE_UTILS_BUFSIZE,1,file_source);
          fwrite(aux,FILE_UTILS_BUFSIZE,1,file_target);
          
          slot_counter--;
          
          file_source_size -= FILE_UTILS_BUFSIZE;
    }
    
    char byte;
    
    // ... e o que sobrou da divisão é copiado byte a byte para o arquivo de saída)
    while (file_source_size > 0) {
		
          byte = fgetc(file_source);
          
          if (feof(file_source))
             break;
             
          fputc(byte,file_target);
          file_source_size--;
          
    }
   
    return true;
}

typedef struct file_utils_copy_str {
	
	progressbar* progress;
	size_t* slots_copied;
	unsigned char thread_stop;
	
} file_util_copy_type;

void* file_utils_copy_progress_bar_thread(void* arg) {
	
	file_util_copy_type* thread_params = (file_util_copy_type*) arg;
    
    while (!thread_params->thread_stop) {
		progressbar_update(thread_params->progress,*thread_params->slots_copied);
		usleep(250*1000);
	}

	return NULL;
}

/** Versão da "file_utils_copy" com progressbar. */
boolean file_utils_copy_progress_bar(tipoArquivo file_source, tipoArquivo file_target, size_t file_source_size, const char* progressBarTitle) {
	
	// Aqui calculo a quantidade de "slots" fixos de tamanho 'FILE_UTILS_BUFSIZE' a serem copiados
	boolean status = true;
    char    aux[FILE_UTILS_BUFSIZE];
    size_t  slots_copied, slots_max = (file_source_size / FILE_UTILS_BUFSIZE) + 1;
    
	// Iniciando o progressbar
    pthread_t progressBarThread;
    progressbar *progress = progressbar_new(progressBarTitle,slots_max);
    
    // Preparando parâmetros da thread
    file_util_copy_type thread_params;
    thread_params.progress = progress;
    thread_params.slots_copied = &slots_copied;
    thread_params.thread_stop = 0;
    
    pthread_create(&progressBarThread,NULL,file_utils_copy_progress_bar_thread,&thread_params);
    
    // Aqui começa a cópia de arquivos propriamente dita (primeiro por slots fixos...
    for (slots_copied=0; slots_copied < slots_max-1; slots_copied++) {
		
		fread (aux,FILE_UTILS_BUFSIZE,1,file_source);
		
		// Se ocorrer algum erro na cópia de dados, paro por aqui
		if (fwrite(aux,FILE_UTILS_BUFSIZE,1,file_target) != 1) {
			status = false;
			break;
		}
		
		file_source_size -= FILE_UTILS_BUFSIZE;
    }
    
    thread_params.thread_stop = 1;
    
    // ... aqui, copio os bytes restantes
    fread (aux,file_source_size,1,file_source);
    fwrite(aux,file_source_size,1,file_target);
    
    pthread_join(progressBarThread,NULL);
    
    if ((status) && (file_source_size == 0))
		progressbar_update(progress,slots_max);
	
    progressbar_finish(progress);
   
    return status;
}

/** Lista recursivamente um diretório */
void file_utils_list_dir(char* dir_path) {
	
	tipoDirectory cur_dir_struct = opendir(dir_path);
	nodeDirectory cur_dir_data;
	
	// Variável que armazena o caminho absoluto
	char* full_path_aux;
	
	// Se eu não conseguir abrir o diretório, paro por aqui mesmo
	if (!cur_dir_struct) {
		printf(":: Could not open the directory: '%s'\n",dir_path);
		return;
	}
	
	// Percorrendo o diretório recursivamente e listando arquivos e pastas
	while ((cur_dir_data = readdir(cur_dir_struct)) != NULL) {
		
		// Aqui monto o caminho absoluto para cada entrada encontrada (e pulo os diretórios '.' e '..')
		if (!(str_utils_equals(cur_dir_data->d_name,".") || str_utils_equals(cur_dir_data->d_name,"..")))
			full_path_aux = file_utils_mount_path(2,dir_path,cur_dir_data->d_name);
		else
			continue;

		// Imprimindo nome de diretório ou arquivo
		if (cur_dir_data->d_type == DT_DIR) {
			printf("[DIR ] %s\n", full_path_aux);
			file_utils_list_dir(full_path_aux);
		}
		else if (cur_dir_data->d_type == DT_REG) {
			printf("[FILE] %s\n", full_path_aux);
		}
		
		// Liberando recursos
		free(full_path_aux);
		
	}
	
	closedir(cur_dir_struct);
	
}

/** Subtrai a string 'path_to_remove' a partir do início da string 'full_path'.
 *  Aqui também é ignorado o caracter de separador de arquivo e diretório no início da nova substring. */
const char* file_utils_remove_path(const char* full_path, const char* path_to_remove) {
	
	const char* relative_path = full_path;
	
	// Aqui verifico se 'path_to_remove' está contido em 'full_path'
	if (str_utils_contains(full_path,path_to_remove))
		relative_path += strlen(path_to_remove);
	else
		log_warning("fileUtils.c:","Caminho informado em 'path_to_remove' não está contido em 'full_path'!");
	
	// Removo o caracter separador de arquivo e diretório do início do caminho
	if ((*relative_path == '/') || (*relative_path == '\\'))
		relative_path++;
	
	return relative_path;	
}

/** Prepara a string contendo o caminho relativo de arquivo a ser gravado na mídia binária.
 *  Primeiro monto o caminho relativo usando a funćão 'file_utils_remove_path' e depois con
 *  verto os caracteres separadores de arquivo e diretório para o padrão UNIX '/' */
char* file_utils_prepare_relative_path(const char* full_path, const char* path_to_remove) {
	
	const char* relative_path = file_utils_remove_path(full_path,path_to_remove);
	
	if (!relative_path) {
		
		log_error("fileUtils.c","Falha ao preparar string de caminho relativo!");
		return NULL;
		
	}
	
	char* prepared_path = strdup(relative_path);
	char* path_header   = prepared_path;
	
	for (;*prepared_path != '\0';prepared_path++)
		if (*prepared_path == '\\')
			*prepared_path = '/';
	
	return path_header;
	
}

/** Remove a extensão de um arquivo */
char* file_utils_remove_extension(const char* filename) {
	
	char* aux = strdup(filename);
	size_t i = strlen(aux) - 1UL;
	
	for (; i>0; i--) {
		
		if (filename[i] == '.') {
			aux[i] = '\0';
			break;
		}
		
	}
	
	return aux;
}

/** Cria um arquivo vazio bem como todos os subdiretórios necessários.
 *  Função fortemente inspirada no comando 'touch' do linux. */
tipoArquivo file_utils_touch(const char* full_path) {

	size_t tokens, cur_token = 1, i = 0, j;
	size_t full_path_len = strlen(full_path);
	char* aux = (char*) calloc(FILE_UTILS_BUFSIZE,sizeof(char));
	
	// Pula o primeiro caracter separador de arquivo
	if ((*full_path == '/') || (*full_path == '\\'))
		i++;
	
	// Contando a quantidade de tokens
	for (j=i, tokens=1; j < full_path_len; j++)
		if ((full_path[j] == '/') || (full_path[j] == '\\'))
			tokens++;
	
	// Varrendo o caminho absoluto
	for (; (i < full_path_len) && (cur_token < tokens); i++, cur_token++) {
		
		// Buscando o próximo token
		while ((full_path[i] != '/') && (full_path[i] != '\\') && (i < full_path_len))
			i++;
		
		// Criando as substrings
		strncpy(aux,full_path,i);
		
		// Aqui crio os subdiretórios
		file_utils_create_directory(aux);

	}
	
	free(aux);
	
	// Criando o arquivo vazio
	return fopen(full_path,"w");
}

/** Remove os zeros excedentes de um double */
char* file_utils_remove_trailing_zeroes(double number) {
	
    char *buf = malloc(8);
    char *p;
    
    sprintf(buf, "%.2lf", number);
    p = buf + strlen(buf) - 1;
    
    while (*p == '0' && *p-- != '.');
    *(p+1) = '\0';
    
    if (*p == '.')
		*p = '\0';
    
    return buf;
}

/** Calcula e formata o tamanho de um arquivo */
char* file_utils_readable_file_size(size_t file_size) {
	
    uint8_t i = 0;
    char    buf[15];
    double  file_size_double = (double) file_size;
    
    const static char* units[] = {"B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    
    while (file_size_double >= 1024.0) {
        file_size_double /= 1024.0;
        i++;
    }
    
    char* converted = file_utils_remove_trailing_zeroes(file_size_double);
    sprintf(buf,"%s %s",converted,units[i]);
    
    free(converted);
    
	return strdup(buf);
}

/** Conta quantos arquivos há em um diretório informado */
uint32_t file_utils_dir_count(const char* directory_path) {
	
    tipoDirectory directory;
    nodeDirectory cur_file;
 
	// Aqui realizo a abertura do diretório e,
    directory = opendir(directory_path);
 
	// ...se tudo der certo...
    if (!directory) {
		com_fprintln(stderr,"x Não foi possivel abrir o diretório '%s'!",directory_path);
        return 0;
    }
    
	uint32_t file_counter = 0;
	
	// ...conto quantos arquivos este possui...
    while ((cur_file = readdir(directory)))
		if (cur_file->d_type == DT_REG)
			file_counter++;

	// ...e desaloco os recursos utilizados.
	closedir(directory);
	
	return file_counter;
}
