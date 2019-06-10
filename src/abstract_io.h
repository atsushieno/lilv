
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>

int abstract_fprintf (void* stream, const char *format, ...);


int abstract_ftell(void *stream);
int abstract_fseek(void* stream, long offset, int origin);
void* abstract_readdir(void* path);
const char* abstract_dirent_getname(void* dirent);
void abstract_closedir(void* dir);
void* abstract_opendir(const char* path);
