#ifndef _STRING_UTILS_H_
#define _STRING_UTILS_H_

#ifndef _LIBMEDIA_H_
#include "libmedia.h"
#endif

typedef struct stringList  tipoStringList;
typedef struct stringList* nodeStringList;

extern boolean str_utils_equals(const char* string, const char* another_string);
extern boolean str_utils_is_ascii(char* string);
extern boolean str_utils_is_empty(const char* string);

extern char* str_utils_to_lower(char* string);
extern char* str_utils_reverse (char* string);
extern char* str_utils_trim(const char* string);

extern char* str_utils_get_random_string(size_t size, const char* charset);

extern nodeStringList str_utils_list_create(size_t size, char** data);

extern boolean str_utils_list_contains(const char* string, nodeStringList list, boolean caseSensitive);
extern boolean str_utils_list_free    (nodeStringList list);
extern boolean str_utils_list_print   (nodeStringList list);

extern int str_utils_compare(const char* string01, const char* string02);
extern const char* str_utils_contains(const char* string, const char* substring);
extern boolean str_utils_is_alpha(const char* string);

#endif
