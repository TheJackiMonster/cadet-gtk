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
	
	GtkSizeGroup* sizeGroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	gtk_size_group_add_widget(sizeGroup, header);
	gtk_size_group_add_widget(sizeGroup, content);
}

void cadet_gtk_load_chat(GtkWidget* header, GtkWidget* content, GtkListBoxRow* row) {
	HdyActionRow* contact = HDY_ACTION_ROW(row);
	
	gtk_header_bar_set_subtitle(GTK_HEADER_BAR(header), hdy_action_row_get_title(contact));
	
	gtk_widget_show_all(header);
}
