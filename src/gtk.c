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

#include "msg.h"
#include "gui/util.h"

#include <stdlib.h>

typedef struct chat_state_t {
	gchar name [CGTK_NAME_BUFFER_SIZE];
	
	gboolean use_json;
	gboolean is_group;
} chat_state_t;

static struct {
	config_t config;
	
	cgtk_gui_t gui;
	
	guint idle;
	GHashTable* states;
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

static chat_state_t* CGTK_get_state(GString* key, gboolean* new_entry) {
	chat_state_t* state = (chat_state_t*) g_hash_table_lookup(
			session.states, key
	);
	
	if (state) {
		if (new_entry) *new_entry = FALSE;
		return state;
	} else {
		state = (chat_state_t*) malloc(sizeof(chat_state_t));
		
		memset(state->name, '\0', CGTK_NAME_BUFFER_SIZE);
		
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

static chat_state_t* CGTK_select_state(const char* identity, const char* port) {
	GString* name = CGTK_merge_name(identity, port);
	
	gboolean new_entry;
	chat_state_t* state = CGTK_get_state(name, &new_entry);
	
	if (!new_entry) {
		g_string_free(name, TRUE);
	}
	
	return state;
}

static void CGTK_set_name(const char* identity, const char* port, const char* name) {
	chat_state_t* state = CGTK_select_state(identity, port);
	
	strncpy(state->name, name, CGTK_NAME_BUFFER_SIZE);
	state->name[CGTK_NAME_BUFFER_SIZE - 1] = '\0';
}

static const char* CGTK_get_name(const char* identity, const char* port) {
	chat_state_t* state = CGTK_select_state(identity, port);
	
	return state->name;
}

static void CGTK_set_nick(const char* name) {
	CGTK_set_name(session.gui.attributes.identity, session.config.port, name);
}

static const char* CGTK_get_nick() {
	return CGTK_get_name(session.gui.attributes.identity, session.config.port);
}

static uint8_t CGTK_send_message(const char* destination, const char* port, msg_t* msg) {
	chat_state_t* state = CGTK_select_state(destination, port);
	
	msg->timestamp = time(NULL);
	
	if (msg->kind == MSG_KIND_TALK) {
		msg->sender = CGTK_get_nick();
	} else
	if ((msg->kind == MSG_KIND_JOIN) || (msg->kind == MSG_KIND_LEAVE)) {
		msg->who = CGTK_get_nick();
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
	
	uint8_t result = FALSE;
	
	if ((buffer_len > 0) && (CGTK_send_gnunet_message(messaging, destination, port, buffer, buffer_len) >= buffer_len)) {
		msg->local = TRUE;
		
		if (msg->kind == MSG_KIND_TALK) {
			CGTK_update_chat_ui(&(session.gui), destination, port, msg);
		}
		
		result = TRUE;
	}
	
	if (state->use_json) {
		free((void*) buffer);
	}
	
	return result;
}

static void CGTK_update_host(const char* announce_regex) {
	if (!announce_regex) {
		announce_regex = "\0";
	}
	
	strncpy(session.gui.attributes.regex, announce_regex, CGTK_REGEX_BUFFER_SIZE);
	session.gui.attributes.regex[CGTK_REGEX_BUFFER_SIZE - 1] = '\0';
	
	CGTK_config_update(&(session.gui.config), &(session.config));
	CGTK_send_gnunet_host(messaging, session.config.port, announce_regex);
	
	CGTK_set_nick(session.config.nick);
}

static void CGTK_search_by_name(const char* name) {
	if ((name) && (strlen(name) > 0)) {
		CGTK_send_gnunet_search(messaging, name);
	}
}

static void CGTK_open_group(const char* port) {
	chat_state_t* state = CGTK_select_state(session.gui.attributes.identity, port);
	
	state->use_json = TRUE;
	state->is_group = TRUE;
	
	CGTK_send_gnunet_group(messaging, port);
}

static void CGTK_exit_chat(const char* destination, const char* port) {
	chat_state_t* state = CGTK_select_state(destination, port);
	
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
		} case MSG_GTK_FOUND: {
			const guint hash = CGTK_recv_gnunet_hash(messaging);
			
			const char* identity = CGTK_recv_gnunet_identity(messaging);
			
			if (identity == NULL) {
				CGTK_shutdown("Can't identify search result!\0");
				return FALSE;
			}
			
			CGTK_update_id_search_ui(&(session.gui), hash, identity);
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
			
			chat_state_t* state = CGTK_select_state(session.gui.attributes.identity, port);
			
			CGTK_update_contacts_ui(&(session.gui), source, port, state->is_group? CONTACT_ACTIVE_GROUP : CONTACT_ACTIVE);
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
			
			CGTK_update_contacts_ui(&(session.gui), source, port, CONTACT_INACTIVE);
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
					
					chat_state_t* state = CGTK_select_state(source, port);
					
					state->use_json = msg->decoding? TRUE : FALSE;
					
					CGTK_repair_message(msg, buffer, state->name);
					
					msg->local = FALSE;
					
					CGTK_update_chat_ui(&(session.gui), source, port, msg);
					
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
	CGTK_config_save(&(session.config));
	
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
	
	memset(&(session.gui), 0, sizeof(session.gui));
	
	session.gui.callbacks.set_name = &CGTK_set_name;
	session.gui.callbacks.get_name = &CGTK_get_name;
	session.gui.callbacks.send_message = &CGTK_send_message;
	session.gui.callbacks.update_host = &CGTK_update_host;
	session.gui.callbacks.search_by_name = &CGTK_search_by_name;
	session.gui.callbacks.open_group = &CGTK_open_group;
	session.gui.callbacks.exit_chat = &CGTK_exit_chat;
	
	CGTK_config_load(&(session.config));
	
	memcpy(&(session.gui.config), &(session.config), sizeof(config_t));
	
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
	
	session.states = g_hash_table_new_full(
			(GHashFunc) g_string_hash,
			(GEqualFunc) g_string_equal,
			CGTK_state_key_free,
			CGTK_state_value_free
	);
}
