//
// Created by thejackimonster on 14.04.20.
//

#include "gtk.h"

#include "config.h"
#include "messaging.h"

static messaging_t* messaging;

#include "gui.h"
#include "storage.h"

#ifdef HANDY_USE_ZERO_API
#include <libhandy-0.0/handy.h>
#else
#include <libhandy-1/handy.h>
#endif

#include "msg.h"
#include "gui/files.h"
#include "gui/keys.h"
#include "gui/util.h"

#include <stdlib.h>

static struct {
	config_t config;
	
	cgtk_gui_t gui;
	
	guint idle;
	GHashTable* chats;
} session;

static void CGTK_shutdown(const char* error_message) {
	if (session.gui.id_search.dialog) {
		gtk_widget_destroy(session.gui.id_search.dialog);
	}
	
	if (session.gui.new_contact.dialog) {
		gtk_widget_destroy(session.gui.new_contact.dialog);
	}
	
	if (session.gui.identity.dialog) {
		gtk_widget_destroy(session.gui.identity.dialog);
	}
	
	if (session.gui.management.dialog) {
		gtk_widget_destroy(session.gui.management.dialog);
	}
	
	if (session.gui.main.window) {
		gtk_widget_destroy(session.gui.main.window);
	}
	
	perror(error_message);
}

static void CGTK_string_free(gpointer key) {
	g_string_free((GString*) key, TRUE);
}

static cgtk_chat_t* CGTK_get_chat(GString* id, gboolean* new_entry) {
	cgtk_chat_t* chat = (cgtk_chat_t*) g_hash_table_lookup(
			session.chats, id
	);
	
	if (chat) {
		if (new_entry) *new_entry = FALSE;
		return chat;
	} else {
		chat = (cgtk_chat_t*) malloc(sizeof(cgtk_chat_t));
		
		memset(chat, 0, sizeof(cgtk_chat_t));
		
		chat->files = g_hash_table_new_full((GHashFunc) g_string_hash, (GEqualFunc) g_string_equal, CGTK_string_free, NULL);
		
		if (g_hash_table_insert(session.chats, id, chat)) {
			if (new_entry) *new_entry = TRUE;
			return chat;
		} else {
			free((void*) chat);
			return NULL;
		}
	}
}

static cgtk_chat_t* CGTK_select_chat(const char* identity, const char* port) {
	GString* name = CGTK_merge_name(identity, port);
	
	gboolean new_entry;
	cgtk_chat_t* chat = CGTK_get_chat(name, &new_entry);
	
	if (!new_entry) {
		g_string_free(name, TRUE);
	}
	
	return chat;
}

static void CGTK_set_name(const char* identity, const char* port, const char* name) {
	cgtk_chat_t* chat = CGTK_select_chat(identity, port);
	
	strncpy(chat->name, name, CGTK_NAME_BUFFER_SIZE);
	chat->name[CGTK_NAME_BUFFER_SIZE - 1] = '\0';
	
	CGTK_update_contacts_ui(&(session.gui), identity, port, CONTACT_RELOAD);
}

static const char* CGTK_get_name(const char* identity, const char* port) {
	const cgtk_chat_t* chat = CGTK_select_chat(identity, port);
	
	return chat->name;
}

static void CGTK_set_nick(const char* name) {
	CGTK_set_name(session.gui.attributes.identity, session.config.port, name);
}

static const char* CGTK_get_nick() {
	return CGTK_get_name(session.gui.attributes.identity, session.config.port);
}

static void CGTK_update_host(const char* announce_regex) {
	if (!announce_regex) {
		announce_regex = "\0";
	}
	
	strncpy(session.gui.attributes.regex, announce_regex, CGTK_REGEX_BUFFER_SIZE);
	session.gui.attributes.regex[CGTK_REGEX_BUFFER_SIZE - 1] = '\0';
	
	CGTK_config_update(&(session.gui.config), &(session.config));
	CGTK_send_gnunet_host(messaging, session.config.visibility, session.config.port, announce_regex);
	
	CGTK_set_nick(session.config.nick);
}

static void CGTK_search_by_name(const char* name) {
	if ((name) && (strlen(name) > 0)) {
		CGTK_send_gnunet_search(messaging, name);
	}
}

static void CGTK_open_group(const char* port) {
	cgtk_chat_t* chat = CGTK_select_chat(session.gui.attributes.identity, port);
	
	chat->use_json = TRUE;
	chat->is_group = TRUE;
	
	CGTK_send_gnunet_group(messaging, port);
}

static void CGTK_exit_chat(const char* destination, const char* port) {
	cgtk_chat_t* chat = CGTK_select_chat(destination, port);
	
	chat->use_json = FALSE;
	chat->is_group = FALSE;
	
	CGTK_send_gnunet_exit(messaging, destination, port);
}

static uint8_t CGTK_send_message(const char* destination, const char* port, msg_t* msg) {
	cgtk_chat_t* chat = CGTK_select_chat(destination, port);
	
	msg->timestamp = time(NULL);
	
	if (msg->kind == MSG_KIND_TALK) {
		msg->talk.sender = CGTK_get_nick();
	} else
	if ((msg->kind == MSG_KIND_JOIN) || (msg->kind == MSG_KIND_LEAVE)) {
		msg->join_leave.who = CGTK_get_nick();
	} else
	if (msg->kind == MSG_KIND_FILE) {
		msg->file.publisher = CGTK_get_nick();
	}
	
	if ((msg->kind != MSG_KIND_UNKNOWN) && (msg->kind != MSG_KIND_TALK)) {
		chat->use_json = TRUE;
	}
	
	uint8_t result = FALSE;
	
	if (msg->usage == MSG_USAGE_GLOBAL) {
		size_t buffer_len = 0;
		const char* buffer;
		
		if (chat->use_json) {
			buffer = CGTK_encode_message(msg, &buffer_len);
		} else {
			if ((msg->kind == MSG_KIND_TALK) && (msg->talk.content)) {
				buffer_len = strlen(msg->talk.content);
				buffer = msg->talk.content;
			} else
			if ((msg->kind == MSG_KIND_FILE) && (msg->file.uri)) {
				buffer_len = strlen(msg->file.uri);
				buffer = msg->file.uri;
			}
		}
		
		if ((buffer_len > 0) && (CGTK_send_gnunet_message(messaging, destination, port, buffer, buffer_len) >= buffer_len)) {
			msg->usage = MSG_USAGE_LOCAL;
			
			CGTK_update_chat_ui(&(session.gui), destination, port, msg);
			
			result = TRUE;
		}
		
		if (chat->use_json) {
			free((void*) buffer);
		}
	} else {
		CGTK_update_chat_ui(&(session.gui), destination, port, msg);
		
		result = TRUE;
	}
	
	return result;
}

static void CGTK_upload_file(const char* path) {
	const char* id = gtk_stack_get_visible_child_name(GTK_STACK(session.gui.chat.stack));
	
	GString* name = g_string_new(id);
	
	const char* identity = name->str;
	const char* port = "\0";
	
	uint index = CGTK_split_name(name, &identity, &port);
	
	CGTK_add_file_link_to_chat(&(session.gui), path, identity, port);
	
	name->str[index] = '_';
	
	g_string_free(name, TRUE);
	
	CGTK_send_gnunet_upload(messaging, path);
}

static void CGTK_download_file(const char* uri, const char* path) {
	const char* id = gtk_stack_get_visible_child_name(GTK_STACK(session.gui.chat.stack));
	
	GString* name = g_string_new(id);
	
	const char* identity = name->str;
	const char* port = "\0";
	
	uint index = CGTK_split_name(name, &identity, &port);
	
	CGTK_add_file_link_to_chat(&(session.gui), path, identity, port);
	
	name->str[index] = '_';
	
	g_string_free(name, TRUE);
	
	CGTK_send_gnunet_download(messaging, uri, path);
}

static void CGTK_delete_file(const char* path, gboolean cleanup) {
	cgtk_file_t* file = CGTK_get_file(&(session.gui), path);
	
	msg_t msg = {};
	msg.kind = MSG_KIND_FILE;
	msg.file.hash = file->hash ? file->hash : "\0";
	msg.file.name = file->name ? file->name : "\0";
	
	msg.file.path = path;
	msg.file.progress = -1.0f;
	
	msg.usage = MSG_USAGE_UPDATE;
	
	CGTK_send_message_about_file(&(session.gui), path, &msg);
	
	if (cleanup) {
		CGTK_unload_data_from_file(&(session.gui), path);
	} else {
		file->status = 0.0f;
	}
	
	CGTK_remove_file(path);
}

static void CGTK_unindex_file(const char* path) {
	CGTK_send_gnunet_unindex(messaging, path);
	
	CGTK_delete_file(path, TRUE);
}

static gboolean CGTK_idle(gpointer user_data) {
	msg_type_t type = CGTK_recv_gnunet_msg_type(messaging);
	
	switch (type) {
		case MSG_GUI_IDENTITY: {
			const char *identity = CGTK_recv_gnunet_identity(messaging);
			
			if (!identity) {
				CGTK_shutdown("Can't retrieve identity of peer!\0");
				return FALSE;
			}
			
			CGTK_update_identity_ui(&(session.gui), identity);
			
			if (strlen(CGTK_get_nick()) == 0) {
				CGTK_set_nick(session.config.nick);
				
				GString* regex = CGTK_regex_append_escaped(NULL, CGTK_get_nick());
				
				strncpy(session.gui.attributes.regex, regex->str, CGTK_REGEX_BUFFER_SIZE);
				session.gui.attributes.regex[CGTK_REGEX_BUFFER_SIZE - 1] = '\0';
				
				g_string_free(regex, TRUE);
			}
			
			CGTK_update_host(session.gui.attributes.regex);
			break;
		} case MSG_GUI_FOUND: {
			const guint hash = CGTK_recv_gnunet_hash(messaging);
			
			const char* identity = CGTK_recv_gnunet_identity(messaging);
			
			if (!identity) {
				CGTK_shutdown("Can't identify search result!\0");
				return FALSE;
			}
			
			CGTK_update_id_search_ui(&(session.gui), hash, identity);
			break;
		} case MSG_GUI_CONNECT: {
			const char* source = CGTK_recv_gnunet_identity(messaging);
			
			if (!source) {
				CGTK_shutdown("Can't identify connections source!\0");
				return FALSE;
			}
			
			const char* port = CGTK_recv_gnunet_port(messaging);
			
			if (!port) {
				CGTK_shutdown("Can't identify connections port!\0");
				return FALSE;
			}
			
			cgtk_chat_t* chat = CGTK_select_chat(session.gui.attributes.identity, port);
			
			CGTK_update_contacts_ui(&(session.gui), source, port, CONTACT_ACTIVE);
			break;
		} case MSG_GUI_DISCONNECT: {
			const char* source = CGTK_recv_gnunet_identity(messaging);
			
			if (!source) {
				CGTK_shutdown("Can't identify connections source!\0");
				return FALSE;
			}
			
			const char* port = CGTK_recv_gnunet_port(messaging);
			
			if (!port) {
				CGTK_shutdown("Can't identify connections port!\0");
				return FALSE;
			}
			
			CGTK_update_contacts_ui(&(session.gui), source, port, CONTACT_INACTIVE);
			break;
		} case MSG_GUI_RECV_MESSAGE: {
			const char* source = CGTK_recv_gnunet_identity(messaging);
			
			if (!source) {
				CGTK_shutdown("Can't identify connections source!\0");
				return FALSE;
			}
			
			const char* port = CGTK_recv_gnunet_port(messaging);
			
			if (!port) {
				CGTK_shutdown("Can't identify connections port!\0");
				return FALSE;
			}
			
			size_t length = CGTK_recv_gnunet_msg_length(messaging);
			char buffer[CGTK_MESSAGE_BUFFER_SIZE + 1];
			
			size_t complete = 0;
			
			while (complete < length) {
				ssize_t offset = 0;
				size_t remaining = length - complete;
				
				if (remaining > CGTK_MESSAGE_BUFFER_SIZE) {
					remaining = CGTK_MESSAGE_BUFFER_SIZE;
				}
				
				while (offset < remaining) {
					ssize_t done = CGTK_recv_gnunet_message(messaging, buffer + offset, remaining - offset);
					
					if (done <= 0) {
						CGTK_shutdown("Transmission of message has exited!\0");
						return FALSE;
					}
					
					offset += done;
				}
				
				buffer[offset] = '\0';
				
				if (strlen(buffer) > 0) {
					msg_t *msg = CGTK_decode_message(buffer, offset);
					
					cgtk_chat_t *chat = CGTK_select_chat(source, port);
					
					chat->use_json = msg->decoding ? TRUE : FALSE;
					
					CGTK_repair_message(msg, buffer, offset, chat->name);
					
					if (msg->kind == MSG_KIND_FILE) {
						if (!msg->file.path) {
							const char* extension = msg->file.name? CGTK_get_extension(msg->file.name) : "\0";
							
							msg->file.path = CGTK_generate_download_path(extension);
						}
						
						msg->file.progress = 0.0f;
						
						cgtk_file_t* file = CGTK_get_file(&(session.gui), msg->file.path);
						
						file->name = g_strdup(msg->file.name);
						file->hash = g_strdup(msg->file.hash);
						
						CGTK_add_file_link_to_chat(&(session.gui), msg->file.path, source, port);
					}
					
					CGTK_update_chat_ui(&(session.gui), source, port, msg);
					
					CGTK_free_message(msg);
				}
				
				complete += offset;
			}
			
			break;
		} case MSG_GUI_FILE_PROGRESS: {
			float progress = CGTK_recv_gnunet_progress(messaging);
			
			const char* path = CGTK_recv_gnunet_path(messaging);
			
			if (!path) {
				CGTK_shutdown("Can't identify files path!\0");
				return FALSE;
			}
			
			cgtk_file_t* file = CGTK_get_file(&(session.gui), path);
			
			msg_t msg = {};
			msg.kind = MSG_KIND_FILE;
			msg.file.hash = file->hash? file->hash : "\0";
			msg.file.name = file->name? file->name : "\0";
			
			msg.file.path = path;
			msg.file.progress = progress;
			
			msg.usage = MSG_USAGE_UPDATE;
			
			CGTK_send_message_about_file(&(session.gui), path, &msg);
			break;
		} case MSG_GUI_FILE_COMPLETE: {
			bool upload = CGTK_recv_gnunet_bool(messaging);
			
			const char *path = CGTK_recv_gnunet_path(messaging);
			
			if (!path) {
				CGTK_shutdown("Can't identify files path!\0");
				return FALSE;
			}
			
			GString *spath = g_string_new(path);
			path = spath->str; // copy path before second use of CGTK_recv_gnunet_path(...)
			
			const char *uri = CGTK_recv_gnunet_path(messaging);
			
			if (!uri) {
				g_string_free(spath, TRUE);
				
				CGTK_shutdown("Can't identify files uri!\0");
				return FALSE;
			}
			
			cgtk_file_t *file = CGTK_get_file(&(session.gui), path);
			
			msg_t msg = {};
			msg.kind = MSG_KIND_FILE;
			msg.file.uri = uri;
			msg.file.hash = file->hash ? file->hash : "\0";
			msg.file.name = file->name ? file->name : "\0";
			
			msg.file.path = path;
			msg.file.progress = 1.0f;
			
			msg.usage = upload ? MSG_USAGE_GLOBAL : MSG_USAGE_UPDATE;
			
			CGTK_send_message_about_file(&(session.gui), path, &msg);
			
			g_string_free(spath, TRUE);
			break;
		} case MSG_GUI_FILE_DELETE: {
			const char *path = CGTK_recv_gnunet_path(messaging);
			
			if (!path) {
				CGTK_shutdown("Can't identify files path!\0");
				return FALSE;
			}
			
			CGTK_delete_file(path, TRUE);
			break;
		} case MSG_ERROR: {
			CGTK_shutdown("No connection!\0");
			return FALSE;
		} default: {
			break;
		}
	}
	
	return TRUE;
}

static void CGTK_end_thread(GtkWidget* window, gpointer user_data) {
	CGTK_free_files(&(session.gui));
	
	CGTK_config_save(&(session.config));
	
	if (session.idle) {
		g_source_remove(session.idle);
		session.idle = 0;
	}
	
	if (session.chats) {
		g_hash_table_destroy(session.chats);
		
		session.chats = NULL;
	}
	
	CGTK_close_messaging(messaging);
}

static void CGTK_chat_value_free(gpointer value) {
	cgtk_chat_t* chat = (cgtk_chat_t*) value;
	
	if (chat->members) {
		g_list_free_full(chat->members, g_free);
	}
	
	if (chat->keys_1tu) {
		CGTK_keys_clear(chat);
	}
	
	if (chat->files) {
		g_hash_table_destroy(chat->files);
	}
	
	free(chat);
}

void CGTK_activate_gtk(GtkApplication* application, gpointer user_data) {
	messaging = (messaging_t*) user_data;
	
	memset(&(session.gui), 0, sizeof(session.gui));
	
	session.gui.callbacks.select_chat = &CGTK_select_chat;
	session.gui.callbacks.set_name = &CGTK_set_name;
	session.gui.callbacks.get_name = &CGTK_get_name;
	session.gui.callbacks.update_host = &CGTK_update_host;
	session.gui.callbacks.search_by_name = &CGTK_search_by_name;
	session.gui.callbacks.open_group = &CGTK_open_group;
	session.gui.callbacks.exit_chat = &CGTK_exit_chat;
	session.gui.callbacks.send_message = &CGTK_send_message;
	session.gui.callbacks.upload_file = &CGTK_upload_file;
	session.gui.callbacks.download_file = &CGTK_download_file;
	session.gui.callbacks.delete_file = &CGTK_delete_file;
	session.gui.callbacks.unindex_file = &CGTK_unindex_file;
	
	CGTK_config_load(&(session.config));
	
	memcpy(&(session.gui.config), &(session.config), sizeof(config_t));
	
	CGTK_init_files(&(session.gui));
	
	session.gui.main.window = gtk_application_window_new(application);
	
	gtk_window_set_default_size(GTK_WINDOW(session.gui.main.window), 320, 512);

	CGTK_init_ui(&(session.gui));
	
	g_signal_connect(session.gui.main.window, "destroy\0", G_CALLBACK(CGTK_end_thread), NULL);
	
	gtk_widget_show_all(session.gui.main.window);
	
	#if(CGTK_GTK_SESSION_IDLE_DELAY_MS > 0)
	session.idle = g_timeout_add_full(G_PRIORITY_DEFAULT_IDLE, CGTK_GTK_SESSION_IDLE_DELAY_MS, CGTK_idle, NULL, NULL);
	#else
	session.idle = g_idle_add_full(G_PRIORITY_DEFAULT_IDLE, CGTK_idle, NULL, NULL);
	#endif
	
	session.chats = g_hash_table_new_full(
			(GHashFunc) g_string_hash,
			(GEqualFunc) g_string_equal,
			CGTK_string_free,
			CGTK_chat_value_free
	);
}
