//
// Created by thejackimonster on 12.04.20.
//

#include "chat.h"

#define HANDY_USE_UNSTABLE_API
#include <libhandy-0.0/handy.h>

void cadet_gtk_init_chat(GtkWidget* header, GtkWidget* content, GtkWidget* back_button) {
	gtk_header_bar_set_title(GTK_HEADER_BAR(header), "Chat");
	gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header), TRUE);
	gtk_header_bar_set_has_subtitle(GTK_HEADER_BAR(header), TRUE);
	gtk_header_bar_set_subtitle(GTK_HEADER_BAR(header), "");
	gtk_widget_set_hexpand(header, TRUE);
	
	gtk_container_add(GTK_CONTAINER(header), back_button);
	
	gtk_widget_set_hexpand(content, TRUE);
	gtk_widget_set_vexpand(content, TRUE);
	
	GtkWidget* chat_list = gtk_list_box_new();
	
	gtk_widget_set_hexpand(chat_list, TRUE);
	gtk_widget_set_vexpand(chat_list, TRUE);
	
	GtkWidget* msg_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
	
	GtkWidget* msg_entry = gtk_entry_new();
	gtk_widget_set_hexpand(msg_entry, TRUE);
	
	GtkWidget* msg_button = gtk_button_new_from_icon_name("document-send", GTK_ICON_SIZE_MENU);
	
	gtk_container_add(GTK_CONTAINER(msg_box), msg_entry);
	gtk_container_add(GTK_CONTAINER(msg_box), msg_button);
	
	gtk_container_add(GTK_CONTAINER(content), chat_list);
	gtk_container_add(GTK_CONTAINER(content), msg_box);
	
	GtkSizeGroup* sizeGroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	gtk_size_group_add_widget(sizeGroup, header);
	gtk_size_group_add_widget(sizeGroup, content);
}

void cadet_gtk_load_chat(GtkWidget* header, GtkWidget* content, GtkListBoxRow* row) {
	HdyActionRow* contact = HDY_ACTION_ROW(row);
	
	gtk_header_bar_set_subtitle(GTK_HEADER_BAR(header), hdy_action_row_get_title(contact));
	
	gtk_widget_show_all(header);
}
