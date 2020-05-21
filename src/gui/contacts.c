//
// Created by thejackimonster on 12.04.20.
//

#include "contacts.h"
#include "chat.h"

#ifdef HANDY_USE_ZERO_API
#include <libhandy-0.0/handy.h>
#else
#include <libhandy-1/handy.h>
#endif

#include "util.h"

#include "dialog/contacts_id_search.c"
#include "dialog/contacts_new_contact.c"
#include "dialog/contacts_identity.c"

static void CGTK_activate_contact(GtkListBox* box, GtkListBoxRow* row, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;

	CGTK_load_chat(gui, row);
	
	if (strcmp(hdy_leaflet_get_visible_child_name(HDY_LEAFLET(gui->main.leaflet)), "chat\0") != 0) {
		hdy_leaflet_set_visible_child_name(HDY_LEAFLET(gui->main.leaflet), "chat\0");
	}

#ifdef HANDY_USE_ZERO_API
	gboolean unfolded = (hdy_leaflet_get_fold(HDY_LEAFLET(gui->main.leaflet)) == HDY_FOLD_UNFOLDED);
#else
	gboolean unfolded = !hdy_leaflet_get_folded(HDY_LEAFLET(gui->content_leaflet));
#endif
	
	if (unfolded) {
		gtk_widget_set_visible(gui->chat.back_button, FALSE);
	}
}

void CGTK_init_contacts(GtkWidget* header, GtkWidget* content, cgtk_gui_t* gui) {
	gtk_header_bar_set_title(GTK_HEADER_BAR(header), "Contacts\0");
	gtk_header_bar_set_has_subtitle(GTK_HEADER_BAR(header), FALSE);
	
	gui->contacts.add_button = gtk_button_new_from_icon_name("list-add-symbolic\0", GTK_ICON_SIZE_MENU);
	
	gui->contacts.identity_button = gtk_button_new_from_icon_name("user-info-symbolic\0", GTK_ICON_SIZE_MENU);
	gtk_widget_set_sensitive(gui->contacts.identity_button, FALSE);
	
	gtk_container_add(GTK_CONTAINER(header), gui->contacts.add_button);
	gtk_container_add(GTK_CONTAINER(header), gui->contacts.identity_button);
	
	gui->contacts.list = gtk_list_box_new();
	
	gtk_list_box_set_selection_mode(GTK_LIST_BOX(gui->contacts.list), GTK_SELECTION_BROWSE);
	gtk_widget_set_size_request(gui->contacts.list, 320, 0);
	gtk_widget_set_hexpand(gui->contacts.list, FALSE);
	gtk_widget_set_vexpand(gui->contacts.list, TRUE);
	
	GtkWidget* viewport = gtk_viewport_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(viewport), gui->contacts.list);
	
	GtkWidget* scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(
			GTK_SCROLLED_WINDOW(scrolled),
			GTK_POLICY_NEVER,
			GTK_POLICY_AUTOMATIC
	);
	
	gtk_container_add(GTK_CONTAINER(scrolled), viewport);
	gtk_container_add(GTK_CONTAINER(content), scrolled);
	
	// TODO: load all contacts
	
	GtkSizeGroup* sizeGroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	gtk_size_group_add_widget(sizeGroup, header);
	gtk_size_group_add_widget(sizeGroup, content);
	
	g_signal_connect(gui->contacts.add_button, "clicked\0", G_CALLBACK(CGTK_new_contact_dialog), gui);
	g_signal_connect(gui->contacts.identity_button, "clicked\0", G_CALLBACK(CGTK_identity_dialog), gui);
	g_signal_connect(gui->contacts.list, "row-activated\0", G_CALLBACK(CGTK_activate_contact), gui);
}

void CGTK_open_contact(cgtk_gui_t* gui, const char* identity, const char* port, contact_type_t type) {
	GList *list = gtk_container_get_children(GTK_CONTAINER(gui->contacts.list));
	
	GString* name = CGTK_merge_name(identity, port);
	
	while (list) {
		GtkWidget *row = GTK_WIDGET(list->data);
		
		if (strcmp(gtk_widget_get_name(row), name->str) == 0) {
			hdy_action_row_set_icon_name(HDY_ACTION_ROW(row), "user-available-symbolic\0");
			g_string_free(name, TRUE);
			return;
		}
		
		list = list->next;
	}
	
	HdyActionRow *contact = hdy_action_row_new();
	gtk_widget_set_name(GTK_WIDGET(contact), name->str);
	
	g_string_free(name, TRUE);
	
	name = g_string_new(gui->callbacks.get_name(identity, port));
	
	if (type == CGTK_CONTACT_GROUP) {
		g_string_append(name, " (GROUP)\0");
	}
	
	hdy_action_row_set_title(contact, name->str);
	hdy_action_row_set_subtitle(contact, identity);
	hdy_action_row_set_icon_name(contact, "user-available-symbolic\0");
	
	g_string_free(name, TRUE);
	
	gtk_container_add(GTK_CONTAINER(gui->contacts.list), GTK_WIDGET(contact));
	
	gtk_widget_show_all(GTK_WIDGET(contact));
}

void CGTK_close_contact(cgtk_gui_t* gui, const char* identity, const char* port) {
	GList* list = gtk_container_get_children(GTK_CONTAINER(gui->contacts.list));
	
	GString* name = CGTK_merge_name(identity, port);
	
	while (list) {
		GtkWidget* row = GTK_WIDGET(list->data);
		
		if (strcmp(gtk_widget_get_name(row), name->str) == 0) {
			hdy_action_row_set_icon_name(HDY_ACTION_ROW(row), "user-idle-symbolic\0");
			break;
		}
		
		list = list->next;
	}
	
	g_string_free(name, TRUE);
}

void CGTK_remove_contact(cgtk_gui_t* gui, const char* identity, const char* port) {
	GList* list = gtk_container_get_children(GTK_CONTAINER(gui->contacts.list));
	
	GString* name = CGTK_merge_name(identity, port);
	
	while (list) {
		GtkWidget* row = GTK_WIDGET(list->data);
		
		if (strcmp(gtk_widget_get_name(row), name->str) == 0) {
			CGTK_unload_chat(gui, GTK_LIST_BOX_ROW(row));
			
			gtk_container_remove(GTK_CONTAINER(gui->contacts.list), row);
			break;
		}
		
		list = list->next;
	}
	
	g_string_free(name, TRUE);
	
	const char* swap_chat = gtk_stack_get_visible_child_name(GTK_STACK(gui->chat.stack));
	
	if (swap_chat) {
		list = gtk_container_get_children(GTK_CONTAINER(gui->contacts.list));
		
		while (list) {
			GtkWidget* row = GTK_WIDGET(list->data);
			
			if (strcmp(gtk_widget_get_name(row), swap_chat) == 0) {
				gtk_widget_activate(row);
				break;
			}
			
			list = list->next;
		}
	}
}
