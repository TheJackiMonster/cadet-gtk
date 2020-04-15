//
// Created by thejackimonster on 14.04.20.
//

#include "gtk.h"

#include "messaging.h"

static messaging_t* messaging;

#include "handy_ui.h"
#include "chat.h"

static void CGTK_send_message(GtkWidget* msg_entry, gpointer user_data) {
	GtkWidget* chat_stack = GTK_WIDGET(user_data);
	GtkWidget* chat_list = gtk_stack_get_visible_child(GTK_STACK(chat_stack));
	
	if (gtk_entry_get_text_length(GTK_ENTRY(msg_entry)) > 0) {
		GtkWidget* window = gtk_widget_get_toplevel(chat_list);
		GtkWidget* titleBar = gtk_window_get_titlebar(GTK_WINDOW(window));
		GtkWidget* leaflet = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(titleBar))->data);
		GtkWidget* chat_header = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(leaflet))->next->data);
		
		const char* destination = gtk_header_bar_get_subtitle(GTK_HEADER_BAR(chat_header));
		
		const char* msg_text = gtk_entry_get_text(GTK_ENTRY(msg_entry));
		size_t msg_length = gtk_entry_get_text_length(GTK_ENTRY(msg_entry));
		
		if (CGTK_send_gnunet_message(messaging, destination, msg_text, msg_length) >= 0) {
			CGTK_add_message(chat_list, msg_text, TRUE, "Me");
			
			gtk_entry_set_text(GTK_ENTRY(msg_entry), "");
		}
	}
}

static gboolean CGTK_poll(gpointer user_data) {
	GtkWidget* window = GTK_WIDGET(gtk_window_list_toplevels()->data);
	
	msg_type_t type = CGTK_recv_gnunet_msg_type(messaging);
	
	switch (type) {
		case MSG_GTK_IDENTITY: {
			const char* identity = CGTK_recv_gnunet_identity(messaging);
			
			if (identity == NULL) {
				return FALSE;
			}
			
			CGTK_update_identity_ui(window, identity);
			break;
		} case MSG_GTK_CONNECT: {
			const char* source = CGTK_recv_gnunet_identity(messaging);
			
			if (source == NULL) {
				return FALSE;
			}
			
			CGTK_update_contacts_ui(window, source, "test\0", TRUE);
			break;
		} case MSG_GTK_DISCONNECT: {
			const char* source = CGTK_recv_gnunet_identity(messaging);
			
			if (source == NULL) {
				return FALSE;
			}
			
			CGTK_update_contacts_ui(window, source, "test\0", FALSE);
			break;
		} case MSG_GTK_RECV_MESSAGE: {
			const char *source = CGTK_recv_gnunet_identity(messaging);
			
			if (source == NULL) {
				return FALSE;
			}
			
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
						return FALSE;
					}
					
					offset += done;
				}
				
				buffer[offset] = '\0';
				
				CGTK_update_messages_ui(window, source, "\0", buffer);
				
				complete += offset;
			}
			
			break;
		} case MSG_ERROR: {
			return FALSE;
		} default: {
			break;
		}
	}
	
	return TRUE;
}

static void CGTK_end_thread(GtkWidget* window, gpointer user_data) {
	CGTK_close_messaging(messaging);
}

void CGTK_activate(GtkApplication* application, gpointer user_data) {
	messaging = (messaging_t*) user_data;
	
	GtkWidget* window = gtk_application_window_new(application);
	gtk_window_set_default_size(GTK_WINDOW(window), 320, 512);
	
	handy_callbacks_t callbacks = {
			&CGTK_send_message
	};
	
	CGTK_init_ui(window, callbacks);
	
	g_signal_connect(window, "destroy", G_CALLBACK(CGTK_end_thread), NULL);
	
	gtk_widget_show_all(window);
	
	g_timeout_add_seconds(1, CGTK_poll, NULL);
}