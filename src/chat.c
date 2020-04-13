//
// Created by thejackimonster on 12.04.20.
//

#include "chat.h"

#define HANDY_USE_UNSTABLE_API
#include <libhandy-0.0/handy.h>

void reveal_message(void* data) {
	GtkWidget* revealer = GTK_WIDGET(data);
	guint duration_ms = gtk_revealer_get_transition_duration(GTK_REVEALER(revealer));
	
	g_usleep(1000L * duration_ms);
	
	gtk_revealer_set_reveal_child(GTK_REVEALER(revealer), TRUE);
	pthread_exit(NULL);
}

void send_message(GtkWidget* msg_button, gpointer user_data) {
	GtkWidget* chat_list = GTK_WIDGET(user_data);
	GtkWidget* msg_box = gtk_widget_get_parent(msg_button);
	GtkWidget* msg_entry = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(msg_box))->data);
	
	if (gtk_entry_get_text_length(GTK_ENTRY(msg_entry)) > 0) {
		GtkWidget* revealer = gtk_revealer_new();
		gtk_revealer_set_transition_type(GTK_REVEALER(revealer), GTK_REVEALER_TRANSITION_TYPE_CROSSFADE);
		gtk_revealer_set_transition_duration(GTK_REVEALER(revealer), 250);
		gtk_revealer_set_reveal_child(GTK_REVEALER(revealer), FALSE);
		
		GtkWidget* msg_frame = gtk_frame_new("Me");
		gtk_widget_set_halign(msg_frame, GTK_ALIGN_END);
		gtk_frame_set_label_align(GTK_FRAME(msg_frame), 1.0f, 0.5f);
		gtk_frame_set_shadow_type(GTK_FRAME(msg_frame), GTK_SHADOW_IN);
		gtk_widget_set_size_request(msg_frame, 100, 0);
		gtk_widget_set_valign(msg_frame, GTK_ALIGN_CENTER);
		gtk_widget_set_hexpand(msg_frame, FALSE);
		
		GtkWidget* text = gtk_label_new(gtk_entry_get_text(GTK_ENTRY(msg_entry)));
		gtk_label_set_line_wrap_mode(GTK_LABEL(text), GTK_WRAP_WORD);
		gtk_label_set_line_wrap(GTK_LABEL(text), TRUE);
		gtk_widget_set_halign(text, GTK_ALIGN_START);
		gtk_widget_set_margin_bottom(text, 4);
		gtk_widget_set_margin_start(text, 8);
		gtk_widget_set_margin_top(text, 4);
		gtk_widget_set_margin_end(text, 8);
		
		gtk_container_add(GTK_CONTAINER(msg_frame), text);
		gtk_container_add(GTK_CONTAINER(revealer), msg_frame);
		gtk_container_add(GTK_CONTAINER(chat_list), revealer);
		
		gtk_entry_set_text(GTK_ENTRY(msg_entry), "");
		
		gtk_widget_show_all(revealer);
		
		pthread_t p;
		pthread_create(&p, NULL, reveal_message, revealer);
	}
}

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
	gtk_list_box_set_selection_mode(GTK_LIST_BOX(chat_list), GTK_SELECTION_NONE);
	
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
	
	g_signal_connect(msg_button, "clicked", G_CALLBACK(send_message), chat_list);
}

void cadet_gtk_load_chat(GtkWidget* header, GtkWidget* content, GtkListBoxRow* row) {
	HdyActionRow* contact = HDY_ACTION_ROW(row);
	
	gtk_header_bar_set_subtitle(GTK_HEADER_BAR(header), hdy_action_row_get_title(contact));
	
	gtk_widget_show_all(header);
}
