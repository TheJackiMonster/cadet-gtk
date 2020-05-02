//
// Created by thejackimonster on 14.04.20.
//

#include "gtk.h"

#include "config.h"
#include "messaging.h"

static messaging_t* messaging;

#include "handy_ui.h"

#ifdef HANDY_USE_ZERO_API
#include <libhandy-0.0/handy.h>
#else
#include <libhandy-1/handy.h>
#endif

#include "chat.h"
#include "json.h"

#include <stdlib.h>

typedef struct chat_state_t {
	gboolean use_json;
	gboolean is_group;
} chat_state_t;

static struct {
	guint idle;
	const char* nick;
	GHashTable* states;
} session;

static void CGTK_shutdown(GtkWidget* window, const char* error_message) {
	gtk_widget_destroy(window);
	
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

static uint CGTK_split_name(GString* name, const char** identity, const char** port) {
	size_t index = 0;
	
	*identity = name->str;
	
	while (index < name->len) {
		if (name->str[index] == '_') {
			if (index + 1 < name->len) {
				*port = (name->str + index + 1);
			}
			
			name->str[index] = '\0';
			break;
		}
		
		index++;
	}
	
	return index;
}

static void CGTK_activate_contact(GtkListBox* box, GtkListBoxRow* row, gpointer user_data) {
	GtkWidget* leaflet = gtk_widget_get_parent(GTK_WIDGET(user_data));
	GtkWidget* chat_content = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(leaflet))->next->data);
	GtkWidget* window = gtk_widget_get_toplevel(leaflet);
	GtkWidget* titleBar = gtk_window_get_titlebar(GTK_WINDOW(window));
	GtkWidget* header_leaflet = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(titleBar))->data);
	GtkWidget* header = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(header_leaflet))->next->data);
	
	if (g_regex_match_simple(".*\\((GROUP)\\)", hdy_action_row_get_title(HDY_ACTION_ROW(row)), 0, 0)) {
		GString* name = g_string_new(gtk_widget_get_name(GTK_WIDGET(row)));
		
		gboolean new_entry;
		chat_state_t* state = CGTK_get_state(name, &new_entry);
		
		state->use_json = TRUE;
		state->is_group = TRUE;
		
		const char* destination = name->str;
		const char* port = "\0";
		
		uint index = CGTK_split_name(name, &destination, &port);
		
		msg_t msg = {};
		
		msg.kind = MSG_KIND_JOIN;
		msg.timestamp = time(NULL);
		
		msg.who = session.nick;
		
		size_t buffer_len;
		const char* buffer = CGTK_encode_message(&msg, &buffer_len);
		
		CGTK_send_gnunet_message(messaging, destination, port, buffer, buffer_len);
		
		if (name->str[index] == '\0') {
			name->str[index] = '_';
		}
		
		if (!new_entry) {
			g_string_free(name, TRUE);
		}
		
		free((void*) buffer);
	}
	
	CGTK_load_chat(header, chat_content, row);
	
	if (strcmp(hdy_leaflet_get_visible_child_name(HDY_LEAFLET(leaflet)), "chat\0") != 0) {
		hdy_leaflet_set_visible_child_name(HDY_LEAFLET(leaflet), "chat\0");
	}

#ifdef HANDY_USE_ZERO_API
	gboolean unfolded = (hdy_leaflet_get_fold(HDY_LEAFLET(leaflet)) == HDY_FOLD_UNFOLDED);
#else
	gboolean unfolded = !hdy_leaflet_get_folded(HDY_LEAFLET(leaflet));
#endif
	
	if (unfolded) {
		GtkWidget* back_button = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(header))->data);
		
		gtk_widget_set_visible(back_button, FALSE);
	}
}

static void CGTK_send_message(GtkWidget* msg_entry, gpointer user_data) {
	GtkWidget* window = gtk_widget_get_toplevel(msg_entry);
	GtkWidget* chat_stack = GTK_WIDGET(user_data);
	
	if (gtk_entry_get_text_length(GTK_ENTRY(msg_entry)) > 0) {
		GtkWidget* chat_box = gtk_stack_get_visible_child(GTK_STACK(chat_stack));
		GtkWidget* chat_list = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(chat_box))->next->data);
		
		GString* name = g_string_new(gtk_stack_get_visible_child_name(GTK_STACK(chat_stack)));
		
		gboolean new_entry;
		chat_state_t* state = CGTK_get_state(name, &new_entry);
		
		const char* destination = name->str;
		const char* port = "\0";
		
		uint index = CGTK_split_name(name, &destination, &port);
		
		msg_t msg = {};
		
		msg.kind = MSG_KIND_TALK;
		msg.timestamp = time(NULL);
		
		msg.sender = session.nick;
		msg.content = gtk_entry_get_text(GTK_ENTRY(msg_entry));
		
		size_t buffer_len;
		const char* buffer;
		
		if (state->use_json) {
			buffer = CGTK_encode_message(&msg, &buffer_len);
		} else {
			buffer_len = strlen(msg.content);;
			buffer = msg.content;
		}
		
		if (CGTK_send_gnunet_message(messaging, destination, port, buffer, buffer_len) > 0) {
			msg.local = TRUE;
			
			CGTK_update_messages_ui(window, destination, port, &msg);
			
			gtk_entry_set_text(GTK_ENTRY(msg_entry), "\0");
		}
		
		if (name->str[index] == '\0') {
			name->str[index] = '_';
		}
		
		if (!new_entry) {
			g_string_free(name, TRUE);
		}
		
		if (state->use_json) {
			free((void*) buffer);
		}
	}
}

static void CGTK_set_port(GtkWidget* port_entry, gpointer user_data) {
	const char* port = gtk_entry_get_text(GTK_ENTRY(port_entry));
	
	CGTK_send_gnunet_port(messaging, port);
}

static gboolean CGTK_idle(gpointer user_data) {
	GtkWidget* window = GTK_WIDGET(gtk_window_list_toplevels()->data);
	
	msg_type_t type = CGTK_recv_gnunet_msg_type(messaging);
	
	switch (type) {
		case MSG_GTK_IDENTITY: {
			const char* identity = CGTK_recv_gnunet_identity(messaging);
			
			if (identity == NULL) {
				CGTK_shutdown(window, "Can't retrieve identity of peer!\0");
				return FALSE;
			}
			
			CGTK_update_identity_ui(window, identity);
			break;
		} case MSG_GTK_CONNECT: {
			const char* source = CGTK_recv_gnunet_identity(messaging);
			
			if (source == NULL) {
				CGTK_shutdown(window, "Can't identify connections source!\0");
				return FALSE;
			}
			
			const char* port = CGTK_recv_gnunet_port(messaging);
			
			if (port == NULL) {
				CGTK_shutdown(window, "Can't identify connections port!\0");
				return FALSE;
			}
			
			CGTK_update_contacts_ui(window, source, port, TRUE);
			break;
		} case MSG_GTK_DISCONNECT: {
			const char* source = CGTK_recv_gnunet_identity(messaging);
			
			if (source == NULL) {
				CGTK_shutdown(window, "Can't identify connections source!\0");
				return FALSE;
			}
			
			const char* port = CGTK_recv_gnunet_port(messaging);
			
			if (port == NULL) {
				CGTK_shutdown(window, "Can't identify connections port!\0");
				return FALSE;
			}
			
			CGTK_update_contacts_ui(window, source, port, FALSE);
			break;
		} case MSG_GTK_RECV_MESSAGE: {
			const char *source = CGTK_recv_gnunet_identity(messaging);
			
			printf("A-");
			
			if (source == NULL) {
				CGTK_shutdown(window, "Can't identify connections source!\0");
				return FALSE;
			}
			
			const char* port = CGTK_recv_gnunet_port(messaging);
			
			printf("%s-B-", source);
			
			if (port == NULL) {
				CGTK_shutdown(window, "Can't identify connections port!\0");
				return FALSE;
			}
			
			printf("%s-C-", port);
			
			size_t length = CGTK_recv_gnunet_msg_length(messaging);
			char buffer[60000 + 1];
			
			size_t complete = 0;
			
			while (complete < length) {
				ssize_t offset = 0;
				size_t remaining = length - complete;
				
				if (remaining > 60000) {
					remaining = 60000;
				}
				
				while (offset < remaining) {
					ssize_t done = CGTK_recv_gnunet_message(messaging, buffer + offset, remaining - offset);
					
					if (done <= 0) {
						CGTK_shutdown(window, "Transmission of message has exited!\0");
						return FALSE;
					}
					
					offset += done;
				}
				
				buffer[offset] = '\0';
				
				if (strlen(buffer) > 0) {
					msg_t* msg = CGTK_decode_message(buffer);
					
					if (!(msg->decoding & MSG_DEC_SENDER_BIT)) {
						msg->sender = "other\0";
					}
					
					if (msg->decoding == 0) {
						msg->content = buffer;
					} else
					if (!(msg->decoding & MSG_DEC_CONTENT_BIT)) {
						msg->content = "\0";
					}
					
					msg->local = FALSE;
					
					CGTK_update_messages_ui(window, source, port, msg);
					
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
			CGTK_shutdown(window, "No connection!\0");
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
	
	GtkWidget* window = gtk_application_window_new(application);
	gtk_window_set_default_size(GTK_WINDOW(window), 320, 512);
	
	handy_callbacks_t callbacks;
	
	callbacks.activate_contact = &CGTK_activate_contact;
	callbacks.send_message = &CGTK_send_message;
	callbacks.set_port = &CGTK_set_port;
	
	CGTK_init_ui(window, &callbacks);
	
	g_signal_connect(window, "destroy\0", G_CALLBACK(CGTK_end_thread), NULL);
	
	gtk_widget_show_all(window);
	
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
