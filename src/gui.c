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
#include "gui/keys.h"
#include "gui/notification.h"
#include "gui/util.h"

void CGTK_init_ui(cgtk_gui_t* gui) {
	memset(gui->attributes.identity, '\0', CGTK_IDENTITY_BUFFER_SIZE);
	memset(gui->attributes.regex, '\0', CGTK_REGEX_BUFFER_SIZE);
	
	GtkWidget* contacts_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	
#ifdef HANDY_USE_ZERO_API
	GtkWidget* contacts_header = gtk_header_bar_new();
	
	gui->chat.header = gtk_header_bar_new();
#else
	GtkWidget* contacts_header = hdy_header_bar_new();
	
	gui->chat.header = hdy_header_bar_new();
#endif
	
	GtkWidget* chat_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	
	GtkWidget* title_leaflet = hdy_leaflet_new();
	hdy_leaflet_set_transition_type(HDY_LEAFLET(title_leaflet), HDY_LEAFLET_TRANSITION_TYPE_SLIDE);
	
	gtk_container_add(GTK_CONTAINER(title_leaflet), contacts_header);
	gtk_container_add(GTK_CONTAINER(title_leaflet), gui->chat.header);
	
	gtk_container_child_set(GTK_CONTAINER(title_leaflet), contacts_header, "name\0", "contacts\0", NULL);
	gtk_container_child_set(GTK_CONTAINER(title_leaflet), gui->chat.header, "name\0", "chat\0", NULL);
	hdy_leaflet_set_visible_child_name(HDY_LEAFLET(title_leaflet), "contacts\0");

#ifdef HANDY_USE_ZERO_API
	HdyTitleBar* titleBar = hdy_title_bar_new();
#else
	GtkWidget* titleBar = hdy_title_bar_new();
#endif
	
	gtk_container_add(GTK_CONTAINER(titleBar), title_leaflet);
	
#ifdef HANDY_USE_ZERO_API
	gtk_window_set_titlebar(GTK_WINDOW(gui->main.window), GTK_WIDGET(titleBar));
#else
	gtk_window_set_titlebar(GTK_WINDOW(gui->main.window), titleBar);
#endif
	
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

#ifdef HANDY_USE_ZERO_API
	hdy_header_group_add_header_bar(header_group, GTK_HEADER_BAR(contacts_header));
	hdy_header_group_add_header_bar(header_group, GTK_HEADER_BAR(gui->chat.header));
#else
	hdy_header_group_add_header_bar(header_group, HDY_HEADER_BAR(contacts_header));
	hdy_header_group_add_header_bar(header_group, HDY_HEADER_BAR(gui->chat.header));
#endif
	
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
	
#ifdef HANDY_USE_ZERO_API
	gtk_widget_show_all(GTK_WIDGET(titleBar));
#else
	gtk_widget_show_all(titleBar);
#endif
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
	
	cgtk_chat_t* chat = gui->callbacks.select_chat(identity, port);
	
	switch (msg->kind) {
		case MSG_KIND_TALK: {
			CGTK_add_talk_message(chat_list, msg);
			break;
		} case MSG_KIND_JOIN: {
			CGTK_update_member(chat_list, chat, msg);
			break;
		} case MSG_KIND_LEAVE: {
			CGTK_update_member(chat_list, chat, msg);
			break;
		} case MSG_KIND_INFO: {
			CGTK_update_all_members(chat_list, chat, msg);
			break;
		} case MSG_KIND_FILE: {
			CGTK_add_file_message(gui, chat_list, chat, msg);
			break;
		} case MSG_KIND_KEY: {
			switch (msg->key.type) {
				case MSG_KEY_1TU: {
					CGTK_keys_add(chat, msg->key.data);
					break;
				} case MSG_KEY_GPG: {
					//TODO: Register GPG key to contact or match contact with key and chat
					break;
				} default: {
					break;
				}
			}
			
			break;
		} default: {
			break;
		}
	}
	
	CGTK_notification_from_chat(gui, identity, port, msg);
}
