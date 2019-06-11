
#if ANDROID
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#else
#include <stdio.h>
#include <dirent.h>
#endif

#include "abstract_io.h"

#if ANDROID

AAssetManager *current_asset_manager;

void abstract_set_io_context (void* ioContext)
{
	current_asset_manager = (AAssetManager*) ioContext;
}



int abstract_ftell(void *stream)
{
	AAsset *asset = (AAsset*) stream;
	return AAsset_getLength(asset) - AAsset_getRemainingLength(asset);
}

int abstract_fseek(void* stream, long offset, int origin)
{
	return AAsset_seek ((AAsset*) stream, offset, origin);
}

void abstract_dir_for_each(const char* path,
                  void*       data,
                  void (*f)(const char* path, const char* name, void* data))
{
	AAssetDir *dir = AAssetManager_openDir(current_asset_manager, path);
	do {
		char* file = AAssetDir_getNextFileName(dir);
		if (!file)
			break;
		f(path, file, data);
	} while (1);
}


#else


void abstract_set_io_context (void* ioContext)
{
}



int abstract_ftell(void *stream)
{
	return ftell(stream);
}

int abstract_fseek(void* stream, long offset, int origin)
{
	return fseek(stream, offset, origin);
}


void abstract_dir_for_each(const char* path,
                  void*       data,
                  void (*f)(const char* path, const char* name, void* data))
{
	void* dir = opendir(path);
	if (dir) {
		for (struct dirent* entry; (entry = readdir(dir));) {
			f(path, entry->d_name, data);
		}
		closedir(dir);
	}
}

#endif
