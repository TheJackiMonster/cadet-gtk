//
// Created by thejackimonster on 06.06.20.
//

#include "files.h"

static void CGTK_files_string_key_free(gpointer key) {
	g_string_free((GString*) key, TRUE);
}

static void CGTK_files_image_value_free(gpointer value) {
	g_object_unref(value);
}

static void CGTK_files_description_value_free(gpointer value) {
	cgtk_file_description_t* desc = (cgtk_file_description_t*) value;
	
	if (desc->name) {
		g_free((gpointer) desc->name);
		desc->name = NULL;
	}
	
	if (desc->hash) {
		g_free((gpointer) desc->hash);
		desc->hash = NULL;
	}
	
	g_free(value);
}

void CGTK_init_files(cgtk_files_t* files) {
	files->images = g_hash_table_new_full(
			(GHashFunc) g_string_hash,
			(GEqualFunc) g_string_equal,
			CGTK_files_string_key_free,
			CGTK_files_image_value_free
	);
	
	files->descriptions = g_hash_table_new_full(
			(GHashFunc) g_string_hash,
			(GEqualFunc) g_string_equal,
			CGTK_files_string_key_free,
			CGTK_files_description_value_free
	);
}

static gpointer CGTK_load_data_from_file(cgtk_gui_t* gui, const char* filename) {
	GString* key = g_string_new(filename);
	gpointer result = g_hash_table_lookup(gui->files.images, key);
	
	g_string_free(key, TRUE);
	
	return result;
}

static gboolean CGTK_store_data_to_file(cgtk_gui_t* gui, const char* filename, gpointer data) {
	GString* key = g_string_new(filename);
	gpointer result = g_hash_table_lookup(gui->files.images, key);
	
	if ((!result) || (g_hash_table_remove(gui->files.images, key))) {
		gboolean done = g_hash_table_insert(gui->files.images, key, data);
		
		if (!done) {
			g_string_free(key, TRUE);
		}
		
		return done;
	} else {
		g_string_free(key, TRUE);
		return FALSE;
	}
}

GdkPixbuf* CGTK_load_image_from_file(cgtk_gui_t* gui, const char* filename) {
	return GDK_PIXBUF(CGTK_load_data_from_file(gui, filename));
}

gboolean CGTK_store_image_to_file(cgtk_gui_t* gui, const char* filename, GdkPixbuf* image) {
	return CGTK_store_data_to_file(gui, filename, (gpointer) image);
}

GdkPixbufAnimation* CGTK_load_animation_from_file(cgtk_gui_t* gui, const char* filename) {
	return GDK_PIXBUF_ANIMATION(CGTK_load_data_from_file(gui, filename));
}

static GdkPixbufAnimationIter* current_animation_iter;

GdkPixbufAnimationIter* CGTK_load_animation_iter(GdkPixbufAnimation* animation) {
	static GdkPixbufAnimation* current_animation = NULL;
	
	if (current_animation != animation) {
		if (current_animation_iter) {
			g_object_unref(current_animation_iter);
		}
		
		if (animation) {
			current_animation_iter = gdk_pixbuf_animation_get_iter(animation, NULL);
		} else {
			current_animation_iter = NULL;
		}
	}
	
	current_animation = animation;
	return current_animation_iter;
}

gboolean CGTK_store_animation_to_file(cgtk_gui_t* gui, const char* filename, GdkPixbufAnimation* animation) {
	return CGTK_store_data_to_file(gui, filename, (gpointer) animation);
}

cgtk_file_description_t* CGTK_get_description(cgtk_gui_t* gui, const char* filename) {
	GString* key = g_string_new(filename);
	gpointer result = g_hash_table_lookup(gui->files.descriptions, key);
	
	if (!result) {
		result = g_malloc(sizeof(cgtk_file_description_t));
		memset(result, 0, sizeof(cgtk_file_description_t));
		
		gboolean done = g_hash_table_insert(gui->files.descriptions, key, result);
		
		if (!done) {
			g_string_free(key, TRUE);
			g_free(result);
			result = NULL;
		}
	} else {
		g_string_free(key, TRUE);
	}
	
	return (cgtk_file_description_t*) result;
}

void CGTK_unload_data_from_file(cgtk_gui_t* gui, const char* filename) {
	GString* key = g_string_new(filename);
	g_hash_table_remove(gui->files.images, key);
	g_hash_table_remove(gui->files.descriptions, key);
	g_string_free(key, TRUE);
}

void CGTK_free_files(cgtk_files_t* files) {
	if (current_animation_iter) {
		g_object_unref(current_animation_iter);
		current_animation_iter = NULL;
	}
	
	if (files->images) {
		g_hash_table_destroy(files->images);
		
		files->images = NULL;
	}
	
	if (files->descriptions) {
		g_hash_table_destroy(files->descriptions);
		
		files->descriptions = NULL;
	}
}
