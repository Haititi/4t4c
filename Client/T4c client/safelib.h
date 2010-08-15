#ifndef SAFELIB_H
#define SAFELIB_H


void *SAFE_realloc (void *allocation, int old_count, int new_count, size_t element_size, bool cleanup);
void SAFE_free (void **address_of_pointer_to_allocation);
void *SAFE_malloc (int count, size_t element_size, bool cleanup);
char *SAFE_strncpy (char *destination, const char *source, size_t buffer_size);
int SAFE_snprintf (char *destination, size_t buffer_size, const char *fmt, ...);
int SAFE_vsnprintf (char *destination, size_t buffer_size, const char *fmt, va_list argptr);
char *SAFE_strncatf (char *destination, size_t buffer_size, const char *fmt, ...);


#endif