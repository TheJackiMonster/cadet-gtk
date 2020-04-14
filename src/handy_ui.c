//
// Created by thejackimonster on 12.04.20.
//

#include "handy_ui.h"

#define HANDY_USE_UNSTABLE_API
#include <libhandy-0.0/handy.h>

#include "contacts.h"
#include "chat.h"

static void CGTK_back(GtkWidget* back_button, gpointer user_data) {
	GtkWidget* content_leaflet = GTK_WIDGET(user_data);
	
	hdy_leaflet_set_visible_child_name(HDY_LEAFLET(content_leaflet), "contacts");
}

void CGTK_init_ui(GtkWidget* window) {
	GtkWidget* contacts_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget* contacts_header = gtk_header_bar_new();
	
	GtkWidget* contacts_list = gtk_list_box_new();
	
	CGTK_init_contacts(contacts_header, contacts_box, contacts_list);
	
	GtkWidget* id_button = gtk_button_new_from_icon_name("avatar-default", GTK_ICON_SIZE_MENU);
	gtk_widget_set_sensitive(id_button, FALSE);
	
	gtk_container_add(GTK_CONTAINER(contacts_header), id_button);
	
	GtkWidget* chat_header = gtk_header_bar_new();
	GtkWidget* chat_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget* back_button = gtk_button_new_from_icon_name("go-previous-symbolic", GTK_ICON_SIZE_MENU);
	
	CGTK_init_chat(chat_header, chat_box, back_button);
	
	GtkWidget* title_leaflet = hdy_leaflet_new();
	hdy_leaflet_set_transition_type(HDY_LEAFLET(title_leaflet), HDY_LEAFLET_TRANSITION_TYPE_SLIDE);
	
	gtk_container_add(GTK_CONTAINER(title_leaflet), contacts_header);
	gtk_container_add(GTK_CONTAINER(title_leaflet), chat_header);
	
	gtk_container_child_set(GTK_CONTAINER(title_leaflet), contacts_header, "name", "contacts", NULL);
	gtk_container_child_set(GTK_CONTAINER(title_leaflet), chat_header, "name", "chat", NULL);
	hdy_leaflet_set_visible_child_name(HDY_LEAFLET(title_leaflet), "contacts");
	
	GtkWidget* content_leaflet = hdy_leaflet_new();
	hdy_leaflet_set_transition_type(HDY_LEAFLET(content_leaflet), HDY_LEAFLET_TRANSITION_TYPE_SLIDE);
	
	gtk_container_add(GTK_CONTAINER(content_leaflet), contacts_box);
	gtk_container_add(GTK_CONTAINER(content_leaflet), chat_box);
	
	gtk_container_child_set(GTK_CONTAINER(content_leaflet), contacts_box, "name", "contacts", NULL);
	gtk_container_child_set(GTK_CONTAINER(content_leaflet), chat_box, "name", "chat", NULL);
	hdy_leaflet_set_visible_child_name(HDY_LEAFLET(content_leaflet), "contacts");
	
	HdyTitleBar* titleBar = hdy_title_bar_new();
	
	gtk_container_add(GTK_CONTAINER(titleBar), title_leaflet);
	
	gtk_window_set_titlebar(GTK_WINDOW(window), GTK_WIDGET(titleBar));
	gtk_container_add(GTK_CONTAINER(window), content_leaflet);
	
	HdyHeaderGroup* header_group = hdy_header_group_new();
	hdy_header_group_add_header_bar(header_group, GTK_HEADER_BAR(contacts_header));
	hdy_header_group_add_header_bar(header_group, GTK_HEADER_BAR(chat_header));
	
	g_object_bind_property(
			content_leaflet,
			"visible-child-name",
			title_leaflet,
			"visible-child-name",
			G_BINDING_SYNC_CREATE
	);
	
	g_object_bind_property(
			title_leaflet,
			"folded",
			back_button,
			"visible",
			G_BINDING_SYNC_CREATE
	);
	
	g_signal_connect(back_button, "clicked", G_CALLBACK(CGTK_back), content_leaflet);
}

static void CGTK_open_identity(GtkWidget* id_button, gpointer user_data) {
	const char* identity = *((const char**) user_data);
	
	GtkWidget* window = gtk_widget_get_toplevel(id_button);
	
	GtkWidget* dialog = hdy_dialog_new(GTK_WINDOW(window));
	gtk_window_set_title(GTK_WINDOW(dialog), "Identity");
	gtk_widget_set_size_request(dialog, 320, 0);
	
	GtkWidget* main_box = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(dialog))->data);
	gtk_box_set_spacing(GTK_BOX(main_box), 2);
	gtk_widget_set_margin_start(main_box, 4);
	gtk_widget_set_margin_bottom(main_box, 4);
	gtk_widget_set_margin_end(main_box, 4);
	gtk_widget_set_margin_top(main_box, 4);
	gtk_widget_set_vexpand(main_box, TRUE);
	
	//GtkWidget* id_label = gtk_label_new(identity);
	
	//gtk_container_add(GTK_CONTAINER(main_box), id_label);
	
	printf("Hello world\n");
	
	gtk_widget_show_all(dialog);
}

void CGTK_update_identity_ui(GtkWidget* window, const char* identity) {
	static gulong handler_id = 0;
	
	GtkWidget* titleBar = gtk_window_get_titlebar(GTK_WINDOW(window));
	GtkWidget* leaflet = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(titleBar))->data);
	GtkWidget* contacts_header = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(leaflet))->data);
	GtkWidget* id_button = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(contacts_header))->next->data);
	
	gtk_widget_set_sensitive(id_button, TRUE);
	gtk_widget_show_all(id_button);
	
	if (handler_id != 0) {
		g_signal_handler_disconnect(id_button, handler_id);
	}
	
	handler_id = g_signal_connect(id_button, "clicked", G_CALLBACK(CGTK_open_identity), &identity);
}
