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
#include "gui/util.h"

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
		name = CGTK_get_entry_text(gui->id_search.entry);
		
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
			CGTK_open_contact(gui, identity, port);
			break;
		} case CONTACT_RELOAD: {
			CGTK_reload_contact(gui, identity, port);
			break;
		} default: {
			break;
		}
	}
}

void CGTK_update_chat_ui(cgtk_gui_t* gui, const char* identity, const char* port, const msg_t* msg) {
	GtkWidget* chat_list = CGTK_get_chat_list(gui, identity, port);
	GtkWidget* port_label = CGTK_get_chat_label(gui, identity, port);
	
	cgtk_chat_t* chat = gui->callbacks.select_chat(identity, port);
	
	switch (msg->kind) {
		case MSG_KIND_TALK: {
			CGTK_add_message(chat_list, msg);
			break;
		} case MSG_KIND_JOIN: {
			cgtk_member_t* member = (cgtk_member_t*) g_malloc(sizeof(cgtk_member_t));
			
			memset(member, 0, sizeof(cgtk_member_t));
			
			strncpy(member->name, msg->who, CGTK_NAME_BUFFER_SIZE);
			member->name[CGTK_NAME_BUFFER_SIZE - 1] = '\0';
			
			printf("join: %s\n", member->name);
			
			chat->members = g_list_append(chat->members, member);
			break;
		} case MSG_KIND_LEAVE: {
			GList* filtered = NULL;
			GList* iter = chat->members;
			
			while (iter) {
				cgtk_member_t* member = (cgtk_member_t*) iter->data;
				
				if (strcmp(member->name, msg->who) == 0) {
					printf("leave: %s\n", member->name);
					
					g_free(member);
				} else {
					filtered = g_list_append(filtered, member);
				}
				
				iter = iter->next;
			}
			
			if (chat->members) {
				g_list_free(chat->members);
			}
			
			chat->members = filtered;
			break;
		} case MSG_KIND_INFO: {
			if (chat->members) {
				g_list_free_full(chat->members, g_free);
				chat->members = NULL;
			}
			
			const char** part = msg->participants;
			
			while (*part) {
				cgtk_member_t* member = (cgtk_member_t*) g_malloc(sizeof(cgtk_member_t));
				
				memset(member, 0, sizeof(cgtk_member_t));
				
				strncpy(member->name, *part, CGTK_NAME_BUFFER_SIZE);
				member->name[CGTK_NAME_BUFFER_SIZE - 1] = '\0';
				
				printf("info: %s\n", member->name);
				
				chat->members = g_list_append(chat->members, member);
				part++;
			}
			
			break;
		} default: {
			break;
		}
	}
}
