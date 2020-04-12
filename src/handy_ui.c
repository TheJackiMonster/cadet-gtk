//
// Created by thejackimonster on 12.04.20.
//

#include "handy_ui.h"

#define HANDY_USE_UNSTABLE_API
#include <libhandy-0.0/handy.h>

#include "contacts.h"
#include "chat.h"

void forward(GtkWidget* button, gpointer user_data) {
	GtkWidget* content_leaflet = GTK_WIDGET(user_data);

	hdy_leaflet_set_visible_child_name(HDY_LEAFLET(content_leaflet), "chat");
}

void back(GtkWidget* back_button, gpointer user_data) {
	GtkWidget* content_leaflet = GTK_WIDGET(user_data);
	
	hdy_leaflet_set_visible_child_name(HDY_LEAFLET(content_leaflet), "contacts");
}

void cadet_gtk_init_ui(GtkWidget* window) {
	GtkWidget* contacts_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget* contacts_header = gtk_header_bar_new();
	
	cadet_gtk_init_contacts(contacts_header, contacts_box);
	
	GtkWidget* fb = gtk_button_new_with_label("Forward");
	
	gtk_container_add(GTK_CONTAINER(contacts_box), fb);
	
	GtkWidget* chat_header = gtk_header_bar_new();
	GtkWidget* chat_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget* back_button = gtk_button_new_from_icon_name("go-previous-symbolic", GTK_ICON_SIZE_MENU);
	
	cadet_gtk_init_chat(chat_header, chat_box, back_button);
	
	GtkWidget* title_leaflet = hdy_leaflet_new();
	hdy_leaflet_set_transition_type(HDY_LEAFLET(title_leaflet), HDY_LEAFLET_TRANSITION_TYPE_SLIDE);
	
	gtk_container_add(GTK_CONTAINER(title_leaflet), contacts_header);
	gtk_container_add(GTK_CONTAINER(title_leaflet), chat_header);
	
	gtk_container_child_set(GTK_CONTAINER(title_leaflet), contacts_header, "name", "contacts", NULL);
	gtk_container_child_set(GTK_CONTAINER(title_leaflet), chat_header, "name", "chat", NULL);
	
	GtkWidget* content_leaflet = hdy_leaflet_new();
	hdy_leaflet_set_transition_type(HDY_LEAFLET(content_leaflet), HDY_LEAFLET_TRANSITION_TYPE_SLIDE);
	
	gtk_container_add(GTK_CONTAINER(content_leaflet), contacts_box);
	gtk_container_add(GTK_CONTAINER(content_leaflet), chat_box);
	
	gtk_container_child_set(GTK_CONTAINER(content_leaflet), contacts_box, "name", "contacts", NULL);
	gtk_container_child_set(GTK_CONTAINER(content_leaflet), chat_box, "name", "chat", NULL);
	
	GtkWidget* titleBar = hdy_title_bar_new();
	
	gtk_container_add(GTK_CONTAINER(titleBar), title_leaflet);
	
	gtk_window_set_titlebar(GTK_WINDOW(window), titleBar);
	gtk_container_add(GTK_CONTAINER(window), content_leaflet);
	
	HdyHeaderGroup* header_group = hdy_header_group_new();
	hdy_header_group_add_header_bar(header_group, contacts_header);
	hdy_header_group_add_header_bar(header_group, chat_header);
	
	g_object_bind_property(
			G_OBJECT(content_leaflet),
			"visible-child-name",
			G_OBJECT(title_leaflet),
			"visible-child-name",
			G_BINDING_SYNC_CREATE
	);
	
	g_object_bind_property(
			G_OBJECT(title_leaflet),
			"folded",
			G_OBJECT(back_button),
			"visible",
			G_BINDING_SYNC_CREATE
	);
	
	g_signal_connect(back_button, "clicked", G_CALLBACK(back), content_leaflet);
	g_signal_connect(fb, "clicked", G_CALLBACK(forward), content_leaflet);
}
