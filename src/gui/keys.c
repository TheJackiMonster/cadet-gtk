//
// Created by thejackimonster on 17.06.20.
//

#include "keys.h"

#include "../storage/files.h"
#include "../storage/keys.h"

static void CGTK_keys_free(gpointer data) {
	cgtk_1tu_key_t* key = (cgtk_1tu_key_t*) data;
	
	CGTK_wipe_key(key);
	g_free(key);
}

void CGTK_keys_add(cgtk_chat_t* chat, const char* data) {
	cgtk_1tu_key_t* key = g_malloc(sizeof(cgtk_1tu_key_t));
	
	if (CGTK_key_from_string(data, key) == 0) {
		chat->keys_1tu = g_list_append(chat->keys_1tu, key);
	} else {
		CGTK_keys_free(key);
	}
}

int CGTK_keys_pick(cgtk_chat_t* chat, const char* path, const cgtk_file_description_t* desc) {
	GList* keys = chat->keys_1tu;
	
	while (keys) {
		cgtk_1tu_key_t* key = keys->data;
		
		if ((CGTK_hash_compare_in_storage(path, key, desc->hash) == 0) && (CGTK_store_key_for(path, key) == 0)) {
			CGTK_keys_free(key);
			
			chat->keys_1tu = g_list_remove_link(chat->keys_1tu, keys);
			return 0;
		}
		
		keys = keys->next;
	}
	
	return -1;
}

void CGTK_keys_clear(cgtk_chat_t* chat) {
	g_list_free_full(chat->keys_1tu, CGTK_keys_free);
	chat->keys_1tu = NULL;
}
