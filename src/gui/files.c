//
// Created by thejackimonster on 06.06.20.
//

#include "files.h"
#include "util.h"

static void CGTK_files_string_free(gpointer key) {
	g_string_free((GString*) key, TRUE);
}

static void CGTK_files_value_free(gpointer value) {
	cgtk_file_t* file = (cgtk_file_t*) value;
	
	if (file->name) {
		g_free((gpointer) file->name);
	}
	
	if (file->hash) {
		g_free((gpointer) file->hash);
	}
	
	if (file->visual_data) {
		g_object_unref(file->visual_data);
	}
	
	if (file->chat_ids) {
		g_list_free_full(file->chat_ids, CGTK_files_string_free);
	}
	
	g_free(value);
}

void CGTK_init_files(cgtk_gui_t* gui) {
	gui->files = g_hash_table_new_full(
			(GHashFunc) g_string_hash,
			(GEqualFunc) g_string_equal,
			CGTK_files_string_free,
			CGTK_files_value_free
	);
}

cgtk_file_t* CGTK_get_file(cgtk_gui_t* gui, const char* filename) {
	GString* key = g_string_new(filename);
	gpointer result = g_hash_table_lookup(gui->files, key);
	
	if (!result) {
		result = g_malloc(sizeof(cgtk_file_t));
		memset(result, 0, sizeof(cgtk_file_t));
		
		cgtk_file_t* file = (cgtk_file_t*) result;
		file->status = 0.0f;
		
		gboolean done = g_hash_table_insert(gui->files, key, result);
		
		if (!done) {
			g_string_free(key, TRUE);
			g_free(result);
			result = NULL;
		}
	} else {
		g_string_free(key, TRUE);
	}
	
	return (cgtk_file_t*) result;
}

static gpointer CGTK_load_visual_data_from_file(cgtk_gui_t* gui, const char* filename) {
	cgtk_file_t* file = CGTK_get_file(gui, filename);
	
	return file? file->visual_data : NULL;
}

static gboolean CGTK_store_visual_data_to_file(cgtk_gui_t* gui, const char* filename, gpointer data) {
	cgtk_file_t* file = CGTK_get_file(gui, filename);
	
	if (file) {
		file->visual_data = data;
		return TRUE;
	} else {
		return FALSE;
	}
}

GdkPixbuf* CGTK_load_image_from_file(cgtk_gui_t* gui, const char* filename) {
	return GDK_PIXBUF(CGTK_load_visual_data_from_file(gui, filename));
}

gboolean CGTK_store_image_to_file(cgtk_gui_t* gui, const char* filename, GdkPixbuf* image) {
	return CGTK_store_visual_data_to_file(gui, filename, (gpointer) image);
}

GdkPixbufAnimation* CGTK_load_animation_from_file(cgtk_gui_t* gui, const char* filename) {
	return GDK_PIXBUF_ANIMATION(CGTK_load_visual_data_from_file(gui, filename));
}

gboolean CGTK_store_animation_to_file(cgtk_gui_t* gui, const char* filename, GdkPixbufAnimation* animation) {
	return CGTK_store_visual_data_to_file(gui, filename, (gpointer) animation);
}

static gint CGTK_compare_file_links(gconstpointer a, gconstpointer b) {
	const GString* as = (const GString*) a;
	const GString* bs = (const GString*) b;
	
	return strcmp(as->str, bs->str);
}

void CGTK_add_file_link_to_chat(cgtk_gui_t* gui, const char* filename, const char* identity, const char* port) {
	cgtk_file_t* file = CGTK_get_file(gui, filename);
	
	if (file) {
		GString* string = CGTK_merge_name(identity, port);
		
		if (g_list_find_custom(file->chat_ids, string, &CGTK_compare_file_links)) {
			g_string_free(string, TRUE);
		} else {
			file->chat_ids = g_list_append(file->chat_ids, string);
		}
	}
}

void CGTK_send_message_about_file(cgtk_gui_t* gui, const char* filename, msg_t* msg) {
	cgtk_file_t* file = CGTK_get_file(gui, filename);
	
	if (file) {
		GList* id = file->chat_ids;
		
		while (id) {
			GString* string = (GString*) id->data;
			
			const char* identity = string->str;
			const char* port = "\0";
			
			uint index = CGTK_split_name(string, &identity, &port);
			
			gui->callbacks.send_message(identity, port, msg);
			
			string->str[index] = '_';
			
			id = id->next;
		}
		
		if ((msg->kind == MSG_KIND_FILE) && (msg->file.progress > file->status)) {
			file->status = msg->file.progress < 1.0f? msg->file.progress : 1.0f;
		}
	}
}

void CGTK_remove_file_link_to_chat(cgtk_gui_t* gui, const char* filename, const char* identity, const char* port) {
	cgtk_file_t* file = CGTK_get_file(gui, filename);
	
	if (file) {
		GString* string = CGTK_merge_name(identity, port);
		GList* match;
		
		do {
			match = g_list_find_custom(file->chat_ids, string, &CGTK_compare_file_links);
			
			if (match) {
				CGTK_files_string_free(match->data);
				
				file->chat_ids = g_list_remove_link(file->chat_ids, match);
			}
		} while (match);
		
		g_string_free(string, TRUE);
	}
}

void CGTK_unload_data_from_file(cgtk_gui_t* gui, const char* filename) {
	GString* key = g_string_new(filename);
	g_hash_table_remove(gui->files, key);
	g_string_free(key, TRUE);
}

void CGTK_free_files(cgtk_gui_t* gui) {
	if (gui->files) {
		g_hash_table_destroy(gui->files);
		
		gui->files = NULL;
	}
}
