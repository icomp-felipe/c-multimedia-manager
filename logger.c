#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void log_error(const char* source_name, const char* format, ...) {

 	va_list args;
 	
 	va_start(args, format);
 	
 	 fprintf(stderr,"[ERR ] %s: ",source_name);
 	vfprintf(stderr, format, args);
 	fputc(0xA,stdout);
 	
 	va_end(args);
	
}

void log_warning(const char* source_name, const char* format, ...) {

 	va_list args;
 	
 	va_start(args, format);
 	
 	 fprintf(stdout,"[WARN] %s: ",source_name);
 	vfprintf(stdout, format, args);
 	fputc(0xA,stdout);
 	
 	va_end(args);
	
}

void log_info(const char* source_name, const char* format, ...) {
	
	va_list args;
 	
 	va_start(args, format);
 	
 	 fprintf(stdout,"[INFO] %s: ",source_name);
 	vfprintf(stdout, format, args);
 	fputc(0xA,stdout);
 	
 	va_end(args);
	
}
