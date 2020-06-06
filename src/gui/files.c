//
// Created by thejackimonster on 06.06.20.
//

#include "files.h"

static void CGTK_files_images_key_free(gpointer key) {
	g_string_free((GString*) key, TRUE);
}

static void CGTK_files_images_value_free(gpointer value) {
	g_object_unref(value);
}

void CGTK_init_files(cgtk_files_t* files) {
	files->images = g_hash_table_new_full(
			(GHashFunc) g_string_hash,
			(GEqualFunc) g_string_equal,
			CGTK_files_images_key_free,
			CGTK_files_images_value_free
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

void CGTK_unload_data_from_file(cgtk_gui_t* gui, const char* filename) {
	GString* key = g_string_new(filename);
	g_hash_table_remove(gui->files.images, key);
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
}
