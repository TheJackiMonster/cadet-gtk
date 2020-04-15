//
// Created by thejackimonster on 12.04.20.
//

#include "chat.h"

#ifdef HANDY_USE_ZERO_API
#include <libhandy-0.0/handy.h>
#else
#include <libhandy-1/handy.h>
#endif

static void CGTK_active_entry(GtkWidget* msg_button, gpointer user_data) {
	GtkWidget* msg_entry = GTK_WIDGET(user_data);
	
	gtk_widget_activate(msg_entry);
}

void CGTK_init_chat(GtkWidget* header, GtkWidget* content, GtkWidget* back_button, handy_callbacks_t callbacks) {
	gtk_header_bar_set_title(GTK_HEADER_BAR(header), "Chat");
	gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header), TRUE);
	gtk_header_bar_set_has_subtitle(GTK_HEADER_BAR(header), TRUE);
	gtk_header_bar_set_subtitle(GTK_HEADER_BAR(header), "");
	gtk_widget_set_hexpand(header, TRUE);
	
	gtk_container_add(GTK_CONTAINER(header), back_button);
	
	gtk_widget_set_hexpand(content, TRUE);
	gtk_widget_set_vexpand(content, TRUE);
	
	GtkWidget* chat_stack = gtk_stack_new();
	
	gtk_widget_set_hexpand(chat_stack, TRUE);
	gtk_widget_set_vexpand(chat_stack, TRUE);
	
	GtkWidget* msg_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
	
	GtkWidget* msg_entry = gtk_entry_new();
	gtk_widget_set_hexpand(msg_entry, TRUE);
	
	GtkWidget* msg_button = gtk_button_new_from_icon_name("document-send", GTK_ICON_SIZE_MENU);
	
	gtk_container_add(GTK_CONTAINER(msg_box), msg_entry);
	gtk_container_add(GTK_CONTAINER(msg_box), msg_button);
	
	gtk_container_add(GTK_CONTAINER(content), chat_stack);
	gtk_container_add(GTK_CONTAINER(content), msg_box);
	
	GtkSizeGroup* sizeGroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	gtk_size_group_add_widget(sizeGroup, header);
	gtk_size_group_add_widget(sizeGroup, content);
	
	g_signal_connect(msg_entry, "activate", G_CALLBACK(callbacks.send_message), chat_stack);
	g_signal_connect(msg_button, "clicked", G_CALLBACK(CGTK_active_entry), msg_entry);
}

GtkWidget* CGTK_get_chat_list(GtkWidget* content, const char* contact_id) {
	GtkWidget* chat_stack = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(content))->data);
	
	GtkWidget* chat_list = gtk_stack_get_child_by_name(GTK_STACK(chat_stack), contact_id);
	
	if (!chat_list) {
		chat_list = gtk_list_box_new();
		
		gtk_list_box_set_selection_mode(GTK_LIST_BOX(chat_list), GTK_SELECTION_NONE);
		
		//
		
		gtk_stack_add_named(GTK_STACK(chat_stack), chat_list, contact_id);
	}
	
	return chat_list;
}

void CGTK_load_chat(GtkWidget* header, GtkWidget* content, GtkListBoxRow* row) {
	GtkWidget* chat_stack = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(content))->data);
	HdyActionRow* contact = HDY_ACTION_ROW(row);
	
	const char* contact_id = hdy_action_row_get_subtitle(contact);
	
	gtk_header_bar_set_subtitle(GTK_HEADER_BAR(header), contact_id);
	
	GtkWidget* chat_list = CGTK_get_chat_list(content, contact_id);
	
	gtk_stack_set_visible_child_name(GTK_STACK(chat_stack), contact_id);
	
	gtk_widget_show_all(chat_stack);
	gtk_widget_show_all(header);
}

static gboolean CGTK_reveal_message(gpointer user_data) {
	GtkWidget* revealer = GTK_WIDGET(user_data);
	
	gtk_revealer_set_reveal_child(GTK_REVEALER(revealer), TRUE);
	
	return FALSE;
}

void CGTK_add_message(GtkWidget* chat_list, const char* msg_text, gboolean my_message, const char* sender) {
	GtkWidget* revealer = gtk_revealer_new();
	
	gtk_revealer_set_transition_type(GTK_REVEALER(revealer),
			(my_message? GTK_REVEALER_TRANSITION_TYPE_SLIDE_LEFT : GTK_REVEALER_TRANSITION_TYPE_SLIDE_RIGHT)
	);
	
	gtk_revealer_set_transition_duration(GTK_REVEALER(revealer), 250);
	gtk_revealer_set_reveal_child(GTK_REVEALER(revealer), FALSE);
	
	GtkWidget* msg_frame = gtk_frame_new(sender);
	gtk_widget_set_halign(msg_frame, (my_message? GTK_ALIGN_END : GTK_ALIGN_START));
	gtk_frame_set_label_align(GTK_FRAME(msg_frame), (my_message? 1.0f : 0.0f), 0.5f);
	gtk_frame_set_shadow_type(GTK_FRAME(msg_frame), GTK_SHADOW_IN);
	gtk_widget_set_size_request(msg_frame, 100, 0);
	gtk_widget_set_valign(msg_frame, GTK_ALIGN_CENTER);
	gtk_widget_set_hexpand(msg_frame, FALSE);
	
	GtkWidget* text = gtk_label_new(msg_text);
	gtk_label_set_line_wrap_mode(GTK_LABEL(text), PANGO_WRAP_WORD);
	gtk_label_set_line_wrap(GTK_LABEL(text), TRUE);
	gtk_widget_set_halign(text, GTK_ALIGN_START);
	gtk_widget_set_margin_bottom(text, 4);
	gtk_widget_set_margin_start(text, 8);
	gtk_widget_set_margin_top(text, 4);
	gtk_widget_set_margin_end(text, 8);
	
	gtk_container_add(GTK_CONTAINER(msg_frame), text);
	gtk_container_add(GTK_CONTAINER(revealer), msg_frame);
	gtk_container_add(GTK_CONTAINER(chat_list), revealer);
	
	guint duration_ms = gtk_revealer_get_transition_duration(GTK_REVEALER(revealer));
	
	gtk_widget_show_all(revealer);
	
	g_timeout_add(duration_ms, CGTK_reveal_message, revealer);
}
