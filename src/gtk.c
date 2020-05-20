//
// Created by thejackimonster on 14.04.20.
//

#include "gtk.h"

#include "config.h"
#include "messaging.h"

static messaging_t* messaging;

#include "gui.h"

#ifdef HANDY_USE_ZERO_API
#include <libhandy-0.0/handy.h>
#else
#include <libhandy-1/handy.h>
#endif

#include "gui/chat.h"
#include "json.h"
#include "gui/util.h"

#include <stdlib.h>

typedef struct chat_state_t {
	gboolean use_json;
	gboolean is_group;
} chat_state_t;

static struct {
	cgtk_gui_t gui;
	
	guint idle;
	const char* nick;
	GHashTable* states;
} session;

static void CGTK_shutdown(const char* error_message) {
	gtk_widget_destroy(session.gui.app_window);
	
	perror(error_message);
}

static chat_state_t* CGTK_get_state(GString* key, gboolean* new_entry) {
	chat_state_t* state = (chat_state_t*) g_hash_table_lookup(
			session.states, key
	);
	
	if (state) {
		if (new_entry) *new_entry = FALSE;
		return state;
	} else {
		state = (chat_state_t*) malloc(sizeof(chat_state_t));
		
		state->use_json = FALSE;
		state->is_group = FALSE;
		
		if (g_hash_table_insert(session.states, key, state)) {
			if (new_entry) *new_entry = TRUE;
			return state;
		} else {
			free((void*) state);
			return NULL;
		}
	}
}

static bool_t CGTK_send_message(const char* destination, const char* port, msg_t* msg) {
	GString* name = CGTK_merge_name(destination, port);
	
	gboolean new_entry;
	chat_state_t* state = CGTK_get_state(name, &new_entry);
	
	msg->timestamp = time(NULL);
	
	if (msg->kind == MSG_KIND_TALK) {
		msg->sender = session.nick;
	} else
	if ((msg->kind == MSG_KIND_JOIN) || (msg->kind == MSG_KIND_LEAVE)) {
		msg->who = session.nick;
	}
	
	size_t buffer_len = 0;
	const char* buffer;
	
	if (state->use_json) {
		buffer = CGTK_encode_message(msg, &buffer_len);
	} else
	if (msg->content) {
		buffer_len = strlen(msg->content);
		buffer = msg->content;
	}
	
	bool_t result = FALSE;
	
	if ((buffer_len > 0) && (CGTK_send_gnunet_message(messaging, destination, port, buffer, buffer_len) >= buffer_len)) {
		msg->local = TRUE;
		
		if (msg->kind == MSG_KIND_TALK) {
			CGTK_update_chat_ui(&(session.gui), destination, port, msg);
		}
		
		result = TRUE;
	}
	
	if (!new_entry) {
		g_string_free(name, TRUE);
	}
	
	if (state->use_json) {
		free((void*) buffer);
	}
	
	return result;
}

static void CGTK_update_host() {
	CGTK_send_gnunet_host(messaging, session.gui.port, session.nick);
}

static void CGTK_search_by_name(const char* name) {
	if ((name) && (strlen(name) > 0)) {
		CGTK_send_gnunet_search(messaging, name);
	}
}

static void CGTK_open_group(const char* port) {
	GString* name = CGTK_merge_name(session.gui.identity, port);
	
	gboolean new_entry;
	chat_state_t* state = CGTK_get_state(name, &new_entry);
	
	if (!new_entry) {
		g_string_free(name, TRUE);
	}
	
	state->use_json = TRUE;
	state->is_group = TRUE;
	
	CGTK_send_gnunet_group(messaging, port);
}

static void CGTK_exit_chat(const char* destination, const char* port) {
	GString* name = CGTK_merge_name(session.gui.identity, port);
	
	gboolean new_entry;
	chat_state_t* state = CGTK_get_state(name, &new_entry);
	
	if (!new_entry) {
		g_string_free(name, TRUE);
	}
	
	state->use_json = FALSE;
	state->is_group = FALSE;
	
	CGTK_send_gnunet_exit(messaging, destination, port);
}

static gboolean CGTK_idle(gpointer user_data) {
	msg_type_t type = CGTK_recv_gnunet_msg_type(messaging);
	
	switch (type) {
		case MSG_GTK_IDENTITY: {
			const char *identity = CGTK_recv_gnunet_identity(messaging);
			
			if (identity == NULL) {
				CGTK_shutdown("Can't retrieve identity of peer!\0");
				return FALSE;
			}
			
			CGTK_update_host();
			
			CGTK_update_identity_ui(&(session.gui), identity);
			break;
		} case MSG_GTK_FOUND: {
			const guint hash = CGTK_recv_gnunet_hash(messaging);
			
			const char* identity = CGTK_recv_gnunet_identity(messaging);
			
			if (identity == NULL) {
				CGTK_shutdown("Can't identify search result!\0");
				return FALSE;
			}
			
			guint cmp_hash = (hash + 1);
			const char* name = NULL;
			
			if (session.gui.search_entry) {
				name = gtk_entry_get_text(GTK_ENTRY(session.gui.search_entry));
				
				GString* search_str = g_string_new(name);
				cmp_hash = g_string_hash(search_str);
				g_string_free(search_str, TRUE);
			}
			
			if ((hash == cmp_hash) && (name) && (session.gui.search_list)) {
				GList *list = gtk_container_get_children(GTK_CONTAINER(session.gui.search_list));
				gboolean duplicate = FALSE;
				
				while (list) {
					GtkWidget *row = GTK_WIDGET(list->data);
					
					if (strcmp(gtk_widget_get_name(row), identity) == 0) {
						duplicate = TRUE;
						break;
					}
					
					list = list->next;
				}
				
				if (!duplicate) {
					HdyActionRow *contact = hdy_action_row_new();
					gtk_widget_set_name(GTK_WIDGET(contact), identity);
					
					hdy_action_row_set_title(contact, name);
					hdy_action_row_set_subtitle(contact, identity);
					hdy_action_row_set_icon_name(contact, "user-available-symbolic\0");
					
					gtk_container_add(GTK_CONTAINER(session.gui.search_list), GTK_WIDGET(contact));
					
					gtk_widget_show_all(GTK_WIDGET(contact));
				}
			}
			
			break;
		} case MSG_GTK_CONNECT: {
			const char* source = CGTK_recv_gnunet_identity(messaging);
			
			if (source == NULL) {
				CGTK_shutdown("Can't identify connections source!\0");
				return FALSE;
			}
			
			const char* port = CGTK_recv_gnunet_port(messaging);
			
			if (port == NULL) {
				CGTK_shutdown("Can't identify connections port!\0");
				return FALSE;
			}
			
			GString* name = CGTK_merge_name(session.gui.identity, port);
			
			gboolean new_entry;
			chat_state_t* state = CGTK_get_state(name, &new_entry);
			
			if (!new_entry) {
				g_string_free(name, TRUE);
			}
			
			CGTK_update_contacts_ui(&(session.gui), source, port, port, state->is_group? CONTACT_ACTIVE_GROUP : CONTACT_ACTIVE);
			break;
		} case MSG_GTK_DISCONNECT: {
			const char* source = CGTK_recv_gnunet_identity(messaging);
			
			if (source == NULL) {
				CGTK_shutdown("Can't identify connections source!\0");
				return FALSE;
			}
			
			const char* port = CGTK_recv_gnunet_port(messaging);
			
			if (port == NULL) {
				CGTK_shutdown("Can't identify connections port!\0");
				return FALSE;
			}
			
			CGTK_update_contacts_ui(&(session.gui), source, port, port, CONTACT_INACTIVE);
			break;
		} case MSG_GTK_RECV_MESSAGE: {
			const char *source = CGTK_recv_gnunet_identity(messaging);
			
			if (source == NULL) {
				CGTK_shutdown("Can't identify connections source!\0");
				return FALSE;
			}
			
			const char* port = CGTK_recv_gnunet_port(messaging);
			
			if (port == NULL) {
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
					msg_t* msg = CGTK_decode_message(buffer, offset);
					
					CGTK_repair_message(msg, buffer);
					
					msg->local = FALSE;
					
					CGTK_update_chat_ui(&(session.gui), source, port, msg);
					
					GString* key = g_string_new(source);
					g_string_append_c_inline(key, '_');
					g_string_append(key, port);
					
					gboolean new_entry;
					chat_state_t* state = CGTK_get_state(key, &new_entry);
					
					if (!new_entry) {
						g_string_free(key, TRUE);
					}
					
					state->use_json = msg->decoding? TRUE : FALSE;
					
					CGTK_free_message(msg);
				}
				
				complete += offset;
			}
			
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
	if (session.idle) {
		g_source_remove(session.idle);
		session.idle = 0;
	}
	
	if (session.states) {
		g_hash_table_destroy(session.states);
		
		session.states = NULL;
	}
	
	CGTK_close_messaging(messaging);
}

void CGTK_state_key_free(gpointer key) {
	g_string_free((GString*) key, TRUE);
}

void CGTK_state_value_free(gpointer value) {
	free(value);
}

void CGTK_activate(GtkApplication* application, gpointer user_data) {
	messaging = (messaging_t*) user_data;
	
	session.gui.callbacks.send_message = &CGTK_send_message;
	session.gui.callbacks.update_host = &CGTK_update_host;
	session.gui.callbacks.search_by_name = &CGTK_search_by_name;
	session.gui.callbacks.open_group = &CGTK_open_group;
	session.gui.callbacks.exit_chat = &CGTK_exit_chat;
	
	session.gui.app_window = gtk_application_window_new(application);
	
	gtk_window_set_default_size(GTK_WINDOW(session.gui.app_window), 320, 512);
	
	CGTK_init_ui(&(session.gui));
	
	g_signal_connect(session.gui.app_window, "destroy\0", G_CALLBACK(CGTK_end_thread), NULL);
	
	gtk_widget_show_all(session.gui.app_window);
	
	#if(CGTK_GTK_SESSION_IDLE_DELAY_MS > 0)
	session.idle = g_timeout_add_full(G_PRIORITY_DEFAULT_IDLE, CGTK_GTK_SESSION_IDLE_DELAY_MS, CGTK_idle, NULL, NULL);
	#else
	session.idle = g_idle_add_full(G_PRIORITY_DEFAULT_IDLE, CGTK_idle, NULL, NULL);
	#endif
	
	session.nick = getenv("USER\0");
	
	if (!session.nick) {
		session.nick = "me\0";
	}
	
	session.states = g_hash_table_new_full(
			(GHashFunc) g_string_hash,
			(GEqualFunc) g_string_equal,
			CGTK_state_key_free,
			CGTK_state_value_free
	);
}
