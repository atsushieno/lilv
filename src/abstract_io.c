
#if ANDROID
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#else
#include <stdio.h>
#include <dirent.h>
#endif

#include "abstract_io.h"

#if ANDROID

int abstract_ftell(void *stream);
int abstract_fseek(void* stream, long offset, int origin);
void* abstract_readdir(const char* path);
const char* abstract_dirent_getname(void* dirent);
void abstract_closedir(void* dir);
void* abstract_opendir(const char* path);

#else



int abstract_fprintf (void* stream, const char *format, ...)
{
    va_list ap;
    va_start (ap, format);
    return fprintf (stream, format, ap);
}



int abstract_ftell(void *stream)
{
	return ftell(stream);
}

int abstract_fseek(void* stream, long offset, int origin)
{
	return fseek(stream, offset, origin);
}

void* abstract_readdir(void *dir)
{
	return readdir((DIR*) dir);
}

const char* abstract_dirent_getname(void* ent)
{
	return ((struct dirent*) ent)->d_name;
}

void abstract_closedir(void* dir)
{
	closedir(dir);
}

void* abstract_opendir(const char* path)
{
	return opendir(path);
}


#endif
