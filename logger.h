#ifndef _LOGGER_H_
#define _LOGGER_H_

void log_error  (const char* source_name, const char* format, ...);
void log_info   (const char* source_name, const char* format, ...);
void log_warning(const char* source_name, const char* format, ...);

#endif
