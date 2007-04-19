/* SLV2
 * Copyright (C) 2007 Dave Robillard <http://drobilla.net>
 *  
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#define _XOPEN_SOURCE 500
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <librdf.h>
#include <slv2/model.h>
#include <slv2/slv2.h>
#include <slv2/util.h>
#include "config.h"
#include "private_types.h"


SLV2Model
slv2_model_new()
{
	struct _Model* model = (struct _Model*)malloc(sizeof(struct _Model));

	model->world = librdf_new_world();
	librdf_world_open(model->world);
	
	model->storage = librdf_new_storage(model->world, "hashes", NULL,
			"hash-type='memory'");

	model->model = librdf_new_model(model->world, model->storage, NULL);

	model->parser = librdf_new_parser(model->world, "turtle", NULL, NULL);

	model->plugins = slv2_plugins_new();

	/*slv2_ontology_uri = raptor_new_uri((const unsigned char*)
		"file://" LV2_TTL_PATH);*/

	return model;
}


void
slv2_model_free(SLV2Model model)
{
	/*raptor_free_uri(slv2_ontology_uri);
	slv2_ontology_uri = NULL;*/

	slv2_plugins_free(model->plugins);
	model->plugins = NULL;
	
	librdf_free_parser(model->parser);
	model->parser = NULL;
	
	librdf_free_model(model->model);
	model->model = NULL;
	
	librdf_free_storage(model->storage);
	model->storage = NULL;
	
	librdf_free_world(model->world);
	model->world = NULL;

	free(model);
}


/* private */
void
slv2_model_load_directory(SLV2Model model, const char* dir)
{
	DIR* pdir = opendir(dir);
	if (!pdir)
		return;
	
	struct dirent* pfile;
	while ((pfile = readdir(pdir))) {
		if (!strcmp(pfile->d_name, ".") || !strcmp(pfile->d_name, ".."))
			continue;

		char* bundle_uri_str = slv2_strjoin("file://", dir, "/", pfile->d_name, "/", NULL);
		librdf_uri* bundle_uri = librdf_new_uri(model->world, (unsigned char*)bundle_uri_str);

		DIR* bundle_dir = opendir(bundle_uri_str + 7);

		if (bundle_dir != NULL) {
			closedir(bundle_dir);

			librdf_uri* manifest_uri = librdf_new_uri_relative_to_base(
					bundle_uri, (const unsigned char*)"manifest.ttl");

			librdf_parser_parse_into_model(model->parser, manifest_uri, NULL, model->model);
			
			librdf_free_uri(manifest_uri);
		}
		
		free(bundle_uri_str);
		librdf_free_uri(bundle_uri);
	}

	closedir(pdir);
}


void
slv2_model_load_path(SLV2Model   model,
                     const char* lv2_path)
{
	char* path = slv2_strjoin(lv2_path, ":", NULL);
	char* dir  = path; // Pointer into path
	
	// Go through string replacing ':' with '\0', using the substring,
	// then replacing it with 'X' and moving on.  i.e. strtok on crack.
	while (strchr(path, ':') != NULL) {
		char* delim = strchr(path, ':');
		*delim = '\0';
		
		slv2_model_load_directory(model, dir);
		
		*delim = 'X';
		dir = delim + 1;
	}
	
	free(path);
}


/** comparator for sorting */
/*int
slv2_plugin_compare_by_uri(const void* a, const void* b)
{
	SLV2Plugin plugin_a = *(SLV2Plugin*)a;
	SLV2Plugin plugin_b = *(SLV2Plugin*)b;

	return strcmp((const char*)librdf_uri_as_string(plugin_a->plugin_uri),
	              (const char*)librdf_uri_as_string(plugin_b->plugin_uri));
}
*/

void
slv2_model_load_all(SLV2Model model)
{
	char* lv2_path = getenv("LV2_PATH");

	// Read all manifest files
	if (lv2_path) {
		slv2_model_load_path(model, lv2_path);
	} else {
		const char* const home = getenv("HOME");
		const char* const suffix = "/.lv2:/usr/local/lib/lv2:usr/lib/lv2";
		lv2_path = slv2_strjoin(home, suffix, NULL);

		//fprintf(stderr, "$LV2_PATH is unset.  Using default path %s\n", lv2_path);

		slv2_model_load_path(model, lv2_path);

		free(lv2_path);
	}

	
	// Find all plugins and associated data files
	unsigned char* query_string = (unsigned char*)
    	"PREFIX : <http://lv2plug.in/ontology#>\n"
		"PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>\n"
		"SELECT DISTINCT ?plugin ?data ?binary\n"
		"WHERE { ?plugin a :Plugin; rdfs:seeAlso ?data\n"
		"OPTIONAL { ?plugin :binary ?binary } }\n"
		"ORDER BY ?plugin\n";
	
	librdf_query* q = librdf_new_query(model->world, "sparql",
		NULL, query_string, NULL);
	
	librdf_query_results* results = librdf_query_execute(q, model->model);

	while (!librdf_query_results_finished(results)) {
	
		librdf_node* plugin_node = librdf_query_results_get_binding_value(results, 0);
		librdf_uri*  plugin_uri  = librdf_node_get_uri(plugin_node);
		librdf_node* data_node   = librdf_query_results_get_binding_value(results, 1);
		librdf_uri*  data_uri    = librdf_node_get_uri(data_node);
		librdf_node* binary_node = librdf_query_results_get_binding_value(results, 2);
		librdf_uri*  binary_uri  = librdf_node_get_uri(binary_node);
		
		SLV2Plugin plugin = slv2_plugins_get_by_uri(model->plugins,
				(const char*)librdf_uri_as_string(plugin_uri));
		
		// Create a new SLV2Plugin
		if (!plugin)
			plugin = slv2_plugin_new(model, plugin_uri,
					(const char*)librdf_uri_as_string(binary_uri));
		
		plugin->model = model;

		// FIXME: check for duplicates
		raptor_sequence_push(plugin->data_uris, librdf_new_uri_from_uri(data_uri));
		
		raptor_sequence_push(model->plugins, plugin);
		
		librdf_free_node(plugin_node);
		librdf_free_node(data_node);

		librdf_query_results_next(results);
	}

	// ORDER BY should (and appears to actually) guarantee this
	//raptor_sequence_sort(model->plugins, slv2_plugin_compare_by_uri);

	if (results)
		librdf_free_query_results(results);
	
	librdf_free_query(q);
}


#if 0
void
slv2_model_serialize(const char* filename)
{
	librdf_uri* lv2_uri = librdf_new_uri(slv2_rdf_world,
			(unsigned char*)"http://lv2plug.in/ontology#");
	
	librdf_uri* rdfs_uri = librdf_new_uri(slv2_rdf_world,
			(unsigned char*)"http://www.w3.org/2000/01/rdf-schema#");

	// Write out test file
	librdf_serializer* serializer = librdf_new_serializer(slv2_rdf_world,
			"turtle", NULL, NULL);
	librdf_serializer_set_namespace(serializer, lv2_uri, "");
	librdf_serializer_set_namespace(serializer, rdfs_uri, "rdfs");
	librdf_serializer_serialize_model_to_file(serializer, filename, NULL, slv2_model);
	librdf_free_serializer(serializer);
}
#endif


SLV2Plugins
slv2_model_get_all_plugins(SLV2Model model)
{
	// FIXME: Slow..
	
	// NULL deleter so user can free returned sequence without nuking
	// our locally stored plugins
	raptor_sequence* result = raptor_new_sequence(NULL, NULL);

	for (int i=0; i < raptor_sequence_size(model->plugins); ++i)
		raptor_sequence_push(result, raptor_sequence_get_at(model->plugins, i));

	// sorted?
	
	return result;
}


SLV2Plugins
slv2_model_get_plugins_by_filter(SLV2Model model, bool (*include)(SLV2Plugin))
{
	SLV2Plugins all = slv2_model_get_all_plugins(model);
	SLV2Plugins result = slv2_plugins_new();

	for (int i=0; i < raptor_sequence_size(all); ++i) {
		SLV2Plugin p = raptor_sequence_get_at(all, i);
		if (include(p))
			raptor_sequence_push(result, slv2_plugin_duplicate(p));
	}

	slv2_plugins_free(all);
	return result;
}


#if 0
SLV2Plugins
slv2_model_get_plugins_by_query(SLV2Model model, const char* query)
{
	SLV2Plugins list = slv2_plugins_new();	

	librdf_query* rq = librdf_new_query(model->world, "sparql",
		NULL, (const unsigned char*)query, NULL);
	
	librdf_query_results* results = librdf_query_execute(rq, model->model);
	
	while (!librdf_query_results_finished(results)) {
		librdf_node* plugin_node = librdf_query_results_get_binding_value(results, 0);
		librdf_uri*  plugin_uri  = librdf_node_get_uri(plugin_node);
		
		SLV2Plugin plugin = slv2_plugins_get_by_uri(list,
				(const char*)librdf_uri_as_string(plugin_uri));

		/* Create a new SLV2Plugin */
		if (!plugin) {
			SLV2Plugin new_plugin = slv2_plugin_new(model, plugin_uri);
			raptor_sequence_push(list, new_plugin);
		}
		
		librdf_free_node(plugin_node);
		
		librdf_query_results_next(results);
	}	
	
	if (results)
		librdf_free_query_results(results);
	
	librdf_free_query(rq);

	return list;
}
#endif
