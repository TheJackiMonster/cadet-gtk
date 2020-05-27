//
// Created by thejackimonster on 12.04.20.
//

#include "gui.h"

#ifdef HANDY_USE_ZERO_API
#include <libhandy-0.0/handy.h>
#else
#include <libhandy-1/handy.h>
#endif

#include "config.h"
#include "gui/contacts.h"
#include "gui/chat.h"

void CGTK_init_ui(cgtk_gui_t* gui) {
	memset(gui->attributes.identity, '\0', CGTK_IDENTITY_BUFFER_SIZE);
	memset(gui->attributes.regex, '\0', CGTK_REGEX_BUFFER_SIZE);
	
	GtkWidget* contacts_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget* contacts_header = gtk_header_bar_new();
	
	gui->chat.header = gtk_header_bar_new();
	GtkWidget* chat_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	
	GtkWidget* title_leaflet = hdy_leaflet_new();
	hdy_leaflet_set_transition_type(HDY_LEAFLET(title_leaflet), HDY_LEAFLET_TRANSITION_TYPE_SLIDE);
	
	gtk_container_add(GTK_CONTAINER(title_leaflet), contacts_header);
	gtk_container_add(GTK_CONTAINER(title_leaflet), gui->chat.header);
	
	gtk_container_child_set(GTK_CONTAINER(title_leaflet), contacts_header, "name\0", "contacts\0", NULL);
	gtk_container_child_set(GTK_CONTAINER(title_leaflet), gui->chat.header, "name\0", "chat\0", NULL);
	hdy_leaflet_set_visible_child_name(HDY_LEAFLET(title_leaflet), "contacts\0");
	
	HdyTitleBar* titleBar = hdy_title_bar_new();
	
	gtk_container_add(GTK_CONTAINER(titleBar), title_leaflet);
	
	gtk_window_set_titlebar(GTK_WINDOW(gui->main.window), GTK_WIDGET(titleBar));
	
	gui->main.leaflet = hdy_leaflet_new();
	hdy_leaflet_set_transition_type(HDY_LEAFLET(gui->main.leaflet), HDY_LEAFLET_TRANSITION_TYPE_SLIDE);
	
	gtk_container_add(GTK_CONTAINER(gui->main.leaflet), contacts_box);
	gtk_container_add(GTK_CONTAINER(gui->main.leaflet), chat_box);
	
	gtk_container_child_set(GTK_CONTAINER(gui->main.leaflet), contacts_box, "name\0", "contacts\0", NULL);
	gtk_container_child_set(GTK_CONTAINER(gui->main.leaflet), chat_box, "name\0", "chat\0", NULL);
	hdy_leaflet_set_visible_child_name(HDY_LEAFLET(gui->main.leaflet), "contacts\0");
	
	gtk_container_add(GTK_CONTAINER(gui->main.window), gui->main.leaflet);
	
	CGTK_init_contacts(contacts_header, contacts_box, gui);
	CGTK_init_chat(gui->chat.header, chat_box, gui);
	
	HdyHeaderGroup* header_group = hdy_header_group_new();
	hdy_header_group_add_header_bar(header_group, GTK_HEADER_BAR(contacts_header));
	hdy_header_group_add_header_bar(header_group, GTK_HEADER_BAR(gui->chat.header));
	
	g_object_bind_property(
			gui->main.leaflet,
			"visible-child-name\0",
			title_leaflet,
			"visible-child-name\0",
			G_BINDING_SYNC_CREATE
	);
	
	g_object_bind_property(
			gui->main.leaflet,
			"folded\0",
			gui->chat.back_button,
			"visible\0",
			G_BINDING_SYNC_CREATE
	);
	
	g_object_bind_property(
			gui->main.leaflet,
			"folded\0",
			contacts_header,
			"show-close-button\0",
			G_BINDING_INVERT_BOOLEAN
	);
	
	g_object_bind_property(
			gui->main.leaflet,
			"folded\0",
			gui->chat.header,
			"show-close-button\0",
			G_BINDING_INVERT_BOOLEAN
	);
	
	gtk_widget_show_all(GTK_WIDGET(titleBar));
}

void CGTK_update_id_search_ui(cgtk_gui_t* gui, guint hash, const char* identity) {
	guint cmp_hash = (hash + 1);
	const char* name = NULL;
	
	if (gui->id_search.entry) {
		name = gtk_entry_get_text(GTK_ENTRY(gui->id_search.entry));
		
		GString* search_str = g_string_new(name);
		cmp_hash = g_string_hash(search_str);
		g_string_free(search_str, TRUE);
	}
	
	if ((hash == cmp_hash) && (name) && (gui->id_search.list)) {
		CGTK_id_search_entry_found(gui, name, identity);
	}
}

void CGTK_update_identity_ui(cgtk_gui_t* gui, const char* identity) {
	strncpy(gui->attributes.identity, identity, CGTK_IDENTITY_BUFFER_SIZE - 1);
	gui->attributes.identity[CGTK_IDENTITY_BUFFER_SIZE - 1] = '\0';
	
	gtk_widget_set_sensitive(gui->contacts.identity_button, TRUE);
}

void CGTK_update_contacts_ui(cgtk_gui_t* gui, const char* identity, const char* port, contact_state_t state) {
	switch (state) {
		case CONTACT_INACTIVE: {
			CGTK_close_contact(gui, identity, port);
			break;
		} case CONTACT_ACTIVE: {
			CGTK_open_contact(gui, identity, port, CGTK_CONTACT_UNKNOWN);
			break;
		} case CONTACT_ACTIVE_GROUP: {
			CGTK_open_contact(gui, identity, port, CGTK_CONTACT_GROUP);
			break;
		} default: {
			break;
		}
	}
}

void CGTK_update_chat_ui(cgtk_gui_t* gui, const char* identity, const char* port, const msg_t* msg) {
	GtkWidget* chat_list = CGTK_get_chat_list(gui, identity, port);
	GtkWidget* port_label = CGTK_get_chat_label(gui, identity, port);
	
	switch (msg->kind) {
		case MSG_KIND_TALK: {
			CGTK_add_message(chat_list, msg);
			break;
		} case MSG_KIND_JOIN: {
			GString* members = g_string_new(gtk_label_get_text(GTK_LABEL(port_label)));
			
			if (members->len > 0) {
				g_string_append(members, ", \0");
			}
			
			g_string_append(members, msg->who);
			
			gtk_label_set_text(GTK_LABEL(port_label), members->str);
			g_string_free(members, TRUE);
			break;
		} case MSG_KIND_LEAVE: {
			GString* members = g_string_new(gtk_label_get_text(GTK_LABEL(port_label)));
			
			const size_t who_len = strlen(msg->who);
			
			char* pos = members->str;
			
			while (pos) {
				pos = strstr(pos, msg->who);
				
				if (pos) {
					if (((pos == msg->who) || (*(pos - 1) == ' ')) &&
						((pos[who_len] == '\0') || ((pos[who_len] == ',') && (pos[who_len + 1] == ' ')))) {
						break;
					} else {
						pos++;
					}
				}
			}
			
			if (pos) {
				if (pos[who_len] == '\0') {
					if (pos == msg->who) {
						g_string_erase(members, 0, who_len);
					} else {
						g_string_erase(members, (ssize_t) (pos - msg->who) - 2, who_len + 2);
					}
				} else {
					g_string_erase(members, (ssize_t) (pos - msg->who), who_len + 2);
				}
				
				gtk_label_set_text(GTK_LABEL(port_label), members->str);
			}
			
			g_string_free(members, TRUE);
			break;
		} case MSG_KIND_INFO: {
			GString* members = g_string_new("\0");
			const char** part = msg->participants;
			
			while (*part) {
				if (members->len > 0) {
					g_string_append(members, ", \0");
				}
				
				g_string_append(members, *part);
				part++;
			}
			
			gtk_label_set_text(GTK_LABEL(port_label), members->str);
			g_string_free(members, TRUE);
			break;
		} default: {
			break;
		}
	}
}
