/*
  Copyright 2020 David Robillard <http://drobilla.net>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include "lilv/lilv.h"

#define MANIFEST_PREFIXES \
	"\
@prefix : <http://example.org/> .\n\
@prefix lv2: <http://lv2plug.in/ns/lv2core#> .\n\
@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#> .\n"

#define PLUGIN_PREFIXES \
	"\
@prefix : <http://example.org/> .\n\
@prefix atom: <http://lv2plug.in/ns/ext/atom#> . \n\
@prefix doap: <http://usefulinc.com/ns/doap#> .\n\
@prefix foaf: <http://xmlns.com/foaf/0.1/> .\n\
@prefix lv2: <http://lv2plug.in/ns/lv2core#> .\n\
@prefix pset: <http://lv2plug.in/ns/ext/presets#> .\n\
@prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> .\n\
@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#> .\n"

#define SIMPLE_MANIFEST_TTL \
	"\
:plug a lv2:Plugin ;\n\
	lv2:binary <foo" SHLIB_EXT "> ;\n\
	rdfs:seeAlso <plugin.ttl> .\n"

#if defined(__APPLE__)
#	define SHLIB_EXT ".dylib"
#elif defined(_WIN32)
#	define SHLIB_EXT ".dll"
#else
#	define SHLIB_EXT ".so"
#endif

typedef struct
{
	LilvWorld* world;
	LilvNode*  plugin1_uri;
	LilvNode*  plugin2_uri;
	char*      test_bundle_path;
	char*      test_bundle_uri;
	char*      test_manifest_path;
	char*      test_content_path;
	int        test_count;
	int        error_count;
} LilvTestEnv;

// Create a new test environment with a world, common values, and test LV2_PATH
LilvTestEnv*
lilv_test_env_new(void);

// Free environment created with lilv_test_env_new()
void
lilv_test_env_free(LilvTestEnv* env);

// Create a bundle with a manifest and plugin files, without loading anything
int
create_bundle(LilvTestEnv* env, const char* manifest, const char* plugin);

// Create a bundle with a manifest and plugin files, then load everything
int
start_bundle(LilvTestEnv* env, const char* manifest, const char* plugin);

// Remove the created bundle from the file system and free its paths in `env`
void
delete_bundle(LilvTestEnv* env);

// Set an environment variable so it is immediately visible in this process
void
set_env(const char* name, const char* value);