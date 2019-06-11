
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
#include <sys/mman.h>

AAssetManager *current_asset_manager;

void abstract_set_io_context (void* ioContext)
{
	current_asset_manager = (AAssetManager*) ioContext;
}



int abstract_ftell(void *stream)
{
	auto asset = (AAsset *) stream;
	return AAsset_getLength(asset) - AAsset_getRemainingLength(asset);
}

int abstract_fseek(void* stream, long offset, int origin)
{
	return AAsset_seek ((AAsset*) stream, offset, origin);
}

void* abstract_readdir(const char* path)
{
}

const char* abstract_dirent_getname(void* dirent)
{
	
}

void abstract_closedir(void* dir)
{
	AAssetDir_close((AAssetDir*) dir);
}

void* abstract_opendir(const char* path)
{
	return AAsset_opendir(mgr, path);
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
