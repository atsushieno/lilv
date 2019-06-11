
#ifndef __ABSTRACT_H_INCLUDED__
#define __ABSTRACT_H_INCLUDED__

#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <lilv/lilv.h>

/* nothing for desktop, AAssetManager* for Android */
LILV_API
void abstract_set_io_context (void* ioContext);

/* lilv-specific */
int abstract_ftell(void *stream);
int abstract_fseek(void* stream, long offset, int origin);


void abstract_dir_for_each(const char* path,
                  void*       data,
                  void (*f)(const char* path, const char* name, void* data));

#endif
