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

void CGTK_init_chat(GtkWidget* header, GtkWidget* content, GtkWidget* back_button, const handy_callbacks_t* callbacks) {
	gtk_header_bar_set_title(GTK_HEADER_BAR(header), "Chat\0");
	gtk_header_bar_set_has_subtitle(GTK_HEADER_BAR(header), TRUE);
	gtk_header_bar_set_subtitle(GTK_HEADER_BAR(header), "\0");
	gtk_widget_set_hexpand(header, TRUE);
	
	GtkWidget* options = gtk_popover_menu_new();
	GtkWidget* options_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
	
	// TODO: add options for different chats
	
	GtkWidget* option_test = gtk_button_new_with_label("Test\0");
	gtk_button_set_relief(GTK_BUTTON(option_test), GTK_RELIEF_NONE);
	
	gtk_container_add(GTK_CONTAINER(options_box), option_test);
	gtk_container_add(GTK_CONTAINER(options), options_box);
	
	gtk_widget_show_all(options_box);
	
	GtkWidget* options_icon = gtk_image_new_from_icon_name("view-more-symbolic\0", GTK_ICON_SIZE_MENU);
	GtkWidget* options_button = gtk_menu_button_new();
	gtk_menu_button_set_popover(GTK_MENU_BUTTON(options_button), options);
	gtk_button_set_image(GTK_BUTTON(options_button), options_icon);
	
	gtk_header_bar_pack_start(GTK_HEADER_BAR(header), back_button);
	gtk_header_bar_pack_end(GTK_HEADER_BAR(header), options_button);
	
	gtk_widget_set_hexpand(content, TRUE);
	gtk_widget_set_vexpand(content, TRUE);
	
	GtkWidget* chat_stack = gtk_stack_new();
	
	gtk_widget_set_hexpand(chat_stack, TRUE);
	gtk_widget_set_vexpand(chat_stack, TRUE);
	
	GtkWidget* msg_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
	
	GtkWidget* msg_entry = gtk_entry_new();
	gtk_widget_set_hexpand(msg_entry, TRUE);
	gtk_widget_set_sensitive(msg_entry, FALSE);
	
	GtkWidget* msg_button = gtk_button_new_from_icon_name("document-send\0", GTK_ICON_SIZE_MENU);
	gtk_widget_set_sensitive(msg_button, FALSE);
	
	gtk_container_add(GTK_CONTAINER(msg_box), msg_entry);
	gtk_container_add(GTK_CONTAINER(msg_box), msg_button);
	
	gtk_container_add(GTK_CONTAINER(content), chat_stack);
	gtk_container_add(GTK_CONTAINER(content), msg_box);
	
	GtkSizeGroup* sizeGroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	gtk_size_group_add_widget(sizeGroup, header);
	gtk_size_group_add_widget(sizeGroup, content);
	
	g_signal_connect(msg_entry, "activate\0", G_CALLBACK(callbacks->send_message), chat_stack);
	g_signal_connect(msg_button, "clicked\0", G_CALLBACK(CGTK_active_entry), msg_entry);
}

GtkWidget* CGTK_get_chat_list(GtkWidget* content, const char* contact_id, const char* contact_port) {
	GtkWidget* chat_stack = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(content))->data);
	
	GString* name = g_string_new(contact_id);
	g_string_append_c(name, '_');
	g_string_append(name, contact_port);
	
	GtkWidget* chat_box = gtk_stack_get_child_by_name(GTK_STACK(chat_stack), name->str);
	GtkWidget* chat_list;
	
	if (!chat_box) {
		chat_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
		
		GtkWidget* port_label = gtk_label_new(contact_port);
		
		chat_list = gtk_list_box_new();
		gtk_list_box_set_selection_mode(GTK_LIST_BOX(chat_list), GTK_SELECTION_NONE);
		
		GtkWidget* viewport = gtk_viewport_new(NULL, NULL);
		gtk_container_add(GTK_CONTAINER(viewport), chat_list);
		
		GtkWidget* scrolled = gtk_scrolled_window_new(NULL, NULL);
		gtk_widget_set_vexpand(scrolled, TRUE);
		gtk_scrolled_window_set_policy(
				GTK_SCROLLED_WINDOW(scrolled),
				GTK_POLICY_NEVER,
				GTK_POLICY_AUTOMATIC
		);
		
		gtk_container_add(GTK_CONTAINER(scrolled), viewport);
		
		gtk_container_add(GTK_CONTAINER(chat_box), port_label);
		gtk_container_add(GTK_CONTAINER(chat_box), scrolled);
		
		gtk_stack_add_named(GTK_STACK(chat_stack), chat_box, name->str);
	} else {
		GtkWidget* scrolled = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(chat_box))->next->data);
		GtkWidget* viewport = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(scrolled))->data);
		
		chat_list = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(viewport))->data);
	}
	
	g_string_free(name, TRUE);
	
	return chat_list;
}

void CGTK_load_chat(GtkWidget* header, GtkWidget* content, GtkListBoxRow* row) {
	GtkWidget* msg_box = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(content))->next->data);
	GtkWidget* msg_entry = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(msg_box))->data);
	GtkWidget* msg_button = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(msg_box))->next->data);
	
	GtkWidget* chat_stack = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(content))->data);
	GString* name = g_string_new(gtk_widget_get_name(GTK_WIDGET(row)));
	
	const char* contact_id = name->str;
	const char* contact_port = "\0";
	
	size_t index = 0;
	
	while (index < name->len) {
		if (name->str[index] == '_') {
			if (index + 1 < name->len) {
				contact_port = (name->str + index + 1);
			}
			
			name->str[index] = '\0';
			break;
		}
		
		index++;
	}
	
	gtk_header_bar_set_subtitle(GTK_HEADER_BAR(header), contact_id);
	
	GtkWidget* chat_list = CGTK_get_chat_list(content, contact_id, contact_port);
	
	if (name->str[index] == '\0') {
		name->str[index] = '_';
	}
	
	gtk_widget_show_all(chat_list);
	gtk_widget_show_all(chat_stack);
	gtk_widget_show_all(header);
	
	gtk_stack_set_visible_child_name(GTK_STACK(chat_stack), name->str);
	
	gtk_widget_set_sensitive(msg_entry, TRUE);
	gtk_widget_set_sensitive(msg_button, TRUE);
	
	g_string_free(name, TRUE);
}

void CGTK_add_message(GtkWidget* chat_list, const msg_t* msg) {
	GtkWidget* msg_frame = gtk_frame_new(msg->sender);
	gtk_widget_set_halign(msg_frame, (msg->local? GTK_ALIGN_END : GTK_ALIGN_START));
	gtk_widget_set_valign(msg_frame, GTK_ALIGN_CENTER);
	gtk_frame_set_label_align(GTK_FRAME(msg_frame), (msg->local? 1.0f : 0.0f), 1.0f);
	gtk_frame_set_shadow_type(GTK_FRAME(msg_frame), GTK_SHADOW_IN);
	gtk_widget_set_size_request(msg_frame, 100, 0);
	gtk_container_set_border_width(GTK_CONTAINER(msg_frame), 4);
	
	GtkWidget* text = gtk_label_new(msg->content);
	gtk_label_set_line_wrap(GTK_LABEL(text), TRUE);
	gtk_label_set_line_wrap_mode(GTK_LABEL(text), PANGO_WRAP_WORD);
	gtk_widget_set_halign(text, GTK_ALIGN_START);
	gtk_widget_set_valign(text, GTK_ALIGN_START);
	gtk_widget_set_margin_bottom(text, 4);
	gtk_widget_set_margin_start(text, 8);
	gtk_widget_set_margin_top(text, 4);
	gtk_widget_set_margin_end(text, 8);
	
	gtk_container_add(GTK_CONTAINER(msg_frame), text);
	gtk_container_add(GTK_CONTAINER(chat_list), msg_frame);
	
	gtk_widget_show_all(chat_list);
}
