//
// Created by thejackimonster on 12.04.20.
//

#include "handy_ui.h"

#ifdef HANDY_USE_ZERO_API
#include <libhandy-0.0/handy.h>
#else
#include <libhandy-1/handy.h>
#endif

#include "contacts.h"
#include "chat.h"

static void CGTK_back(GtkWidget* back_button, gpointer user_data) {
	GtkWidget* content_leaflet = GTK_WIDGET(user_data);
	
	hdy_leaflet_set_visible_child_name(HDY_LEAFLET(content_leaflet), "contacts\0");
}

static char id_buffer [1024];
static char port_buffer [512];

static void CGTK_writeback_port(GtkWidget* id_entry, gpointer user_data) {
	void (*callback)(GtkWidget*, gpointer) = user_data;
	
	GtkWidget* dialog = gtk_widget_get_toplevel(id_entry);
	
	const char* port = gtk_entry_get_text(GTK_ENTRY(id_entry));
	size_t port_length = strlen(port);
	
	strncpy(port_buffer, port, 511);
	id_buffer[511] = '\0';
	
	callback(id_entry, NULL);
	
	gtk_widget_destroy(dialog);
}

static void CGTK_open_identity(GtkWidget* id_button, gpointer user_data) {
	GtkWidget* window = gtk_widget_get_toplevel(id_button);

#ifdef HANDY_USE_ZERO_API
	GtkWidget* dialog = hdy_dialog_new(GTK_WINDOW(window));
#else
	GtkWidget* dialog = gtk_dialog_new();
#endif
	
	gtk_window_set_title(GTK_WINDOW(dialog), "Identity\0");
	gtk_widget_set_size_request(dialog, 300, 0);
	
	GtkWidget* main_box = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(dialog))->data);
	gtk_box_set_spacing(GTK_BOX(main_box), 2);
	gtk_widget_set_margin_start(main_box, 4);
	gtk_widget_set_margin_bottom(main_box, 4);
	gtk_widget_set_margin_end(main_box, 4);
	gtk_widget_set_margin_top(main_box, 4);
	gtk_widget_set_vexpand(main_box, TRUE);

#ifndef HANDY_USE_ZERO_API
	char capitals [8];
	
	for (int i = 0; i < 4; i++) {
		capitals[i*2] = id_buffer[i];
		capitals[i*2 + 1] = (char) (i == 3? '\0' : ' ');
	}
	
	GtkWidget* avatar = hdy_avatar_new(48, capitals, TRUE);
	gtk_widget_set_margin_bottom(avatar, 4);
	gtk_widget_set_margin_start(avatar, 8);
	gtk_widget_set_margin_top(avatar, 4);
	gtk_widget_set_margin_end(avatar, 8);
	
	gtk_container_add(GTK_CONTAINER(main_box), avatar);
#else
	GtkWidget* avatar = gtk_image_new_from_icon_name("avatar-default-symbolic\0", GTK_ICON_SIZE_DIALOG);
	gtk_widget_set_margin_bottom(avatar, 4);
	gtk_widget_set_margin_start(avatar, 8);
	gtk_widget_set_margin_top(avatar, 4);
	gtk_widget_set_margin_end(avatar, 8);
	
	gtk_container_add(GTK_CONTAINER(main_box), avatar);
#endif
	
	GtkWidget* id_label = gtk_label_new(id_buffer);
	gtk_label_set_line_wrap_mode(GTK_LABEL(id_label), PANGO_WRAP_CHAR);
	gtk_label_set_line_wrap(GTK_LABEL(id_label), TRUE);
	gtk_label_set_selectable(GTK_LABEL(id_label), TRUE);
	
	GtkWidget* port_entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(port_entry), port_buffer);
	
	gtk_container_add(GTK_CONTAINER(main_box), id_label);
	gtk_container_add(GTK_CONTAINER(main_box), port_entry);
	
	g_signal_connect(port_entry, "activate\0", G_CALLBACK(CGTK_writeback_port), user_data);
	
	gtk_widget_show_all(dialog);
}

uint CGTK_split_name(GString* name, const char** identity, const char** port) {
	size_t index = 0;
	
	*identity = name->str;
	
	while (index < name->len) {
		if (name->str[index] == '_') {
			if (index + 1 < name->len) {
				*port = (name->str + index + 1);
			}
			
			name->str[index] = '\0';
			break;
		}
		
		index++;
	}
	
	return index;
}

void CGTK_init_ui(GtkWidget* window, const handy_callbacks_t* callbacks) {
	GtkWidget* contacts_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget* contacts_header = gtk_header_bar_new();
	
	memset(port_buffer, '\0', 512);
	
	GtkWidget* contacts_list = gtk_list_box_new();
	
	CGTK_init_contacts(contacts_header, contacts_box, contacts_list, callbacks);
	
	GtkWidget* id_button = gtk_button_new_from_icon_name("user-info-symbolic\0", GTK_ICON_SIZE_MENU);
	gtk_widget_set_sensitive(id_button, FALSE);
	
	gtk_container_add(GTK_CONTAINER(contacts_header), id_button);
	
	GtkWidget* chat_header = gtk_header_bar_new();
	GtkWidget* chat_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget* back_button = gtk_button_new_from_icon_name("go-previous-symbolic\0", GTK_ICON_SIZE_MENU);
	
	CGTK_init_chat(chat_header, chat_box, back_button, callbacks);
	
	GtkWidget* title_leaflet = hdy_leaflet_new();
	hdy_leaflet_set_transition_type(HDY_LEAFLET(title_leaflet), HDY_LEAFLET_TRANSITION_TYPE_SLIDE);
	
	gtk_container_add(GTK_CONTAINER(title_leaflet), contacts_header);
	gtk_container_add(GTK_CONTAINER(title_leaflet), chat_header);
	
	gtk_container_child_set(GTK_CONTAINER(title_leaflet), contacts_header, "name\0", "contacts\0", NULL);
	gtk_container_child_set(GTK_CONTAINER(title_leaflet), chat_header, "name\0", "chat\0", NULL);
	hdy_leaflet_set_visible_child_name(HDY_LEAFLET(title_leaflet), "contacts\0");
	
	GtkWidget* content_leaflet = hdy_leaflet_new();
	hdy_leaflet_set_transition_type(HDY_LEAFLET(content_leaflet), HDY_LEAFLET_TRANSITION_TYPE_SLIDE);
	
	gtk_container_add(GTK_CONTAINER(content_leaflet), contacts_box);
	gtk_container_add(GTK_CONTAINER(content_leaflet), chat_box);
	
	gtk_container_child_set(GTK_CONTAINER(content_leaflet), contacts_box, "name\0", "contacts\0", NULL);
	gtk_container_child_set(GTK_CONTAINER(content_leaflet), chat_box, "name\0", "chat\0", NULL);
	hdy_leaflet_set_visible_child_name(HDY_LEAFLET(content_leaflet), "contacts\0");
	
	HdyTitleBar* titleBar = hdy_title_bar_new();
	
	gtk_container_add(GTK_CONTAINER(titleBar), title_leaflet);
	
	gtk_window_set_titlebar(GTK_WINDOW(window), GTK_WIDGET(titleBar));
	gtk_container_add(GTK_CONTAINER(window), content_leaflet);
	
	HdyHeaderGroup* header_group = hdy_header_group_new();
	hdy_header_group_add_header_bar(header_group, GTK_HEADER_BAR(contacts_header));
	hdy_header_group_add_header_bar(header_group, GTK_HEADER_BAR(chat_header));
	
	g_object_bind_property(
			content_leaflet,
			"visible-child-name\0",
			title_leaflet,
			"visible-child-name\0",
			G_BINDING_SYNC_CREATE
	);
	
	g_object_bind_property(
			title_leaflet,
			"folded\0",
			back_button,
			"visible\0",
			G_BINDING_SYNC_CREATE
	);
	
	g_object_bind_property(
			title_leaflet,
			"folded\0",
			contacts_header,
			"show-close-button\0",
			G_BINDING_INVERT_BOOLEAN
	);
	
	g_object_bind_property(
			title_leaflet,
			"folded\0",
			chat_header,
			"show-close-button\0",
			G_BINDING_INVERT_BOOLEAN
	);
	
	g_signal_connect(back_button, "clicked\0", G_CALLBACK(CGTK_back), content_leaflet);
	g_signal_connect(id_button, "clicked\0", G_CALLBACK(CGTK_open_identity), callbacks->set_port);
	
	gtk_widget_show_all(GTK_WIDGET(titleBar));
}

void CGTK_update_identity_ui(GtkWidget* window, const char* identity) {
	GtkWidget* titleBar = gtk_window_get_titlebar(GTK_WINDOW(window));
	GtkWidget* leaflet = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(titleBar))->data);
	GtkWidget* contacts_header = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(leaflet))->data);
	GtkWidget* id_button = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(contacts_header))->next->data);
	
	strncpy(id_buffer, identity, 1023);
	id_buffer[1023] = '\0';
	
	gtk_widget_set_sensitive(id_button, TRUE);
}

void CGTK_update_contacts_ui(GtkWidget* window, const char* identity, const char* port, gboolean active) {
	GtkWidget* leaflet = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(window))->data);
	GtkWidget* contacts_box = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(leaflet))->data);
	GtkWidget* contacts_list = CGTK_get_contacts_list(contacts_box);
	
	if (active) {
		CGTK_open_contact(contacts_list, identity, port, CGTK_CONTACT_UNKNOWN);
	} else {
		CGTK_close_contact(contacts_list, identity, port);
	}
}

void CGTK_update_messages_ui(GtkWidget* window, const char* identity, const char* port, const msg_t* msg) {
	GtkWidget* leaflet = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(window))->data);
	GtkWidget* chat_box = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(leaflet))->next->data);
	GtkWidget* chat_list = CGTK_get_chat_list(chat_box, identity, port);
	GtkWidget* port_label = GTK_WIDGET(gtk_container_get_children(
			GTK_CONTAINER(gtk_widget_get_parent(chat_list)))->data
	);
	
	switch (msg->kind) {
		case MSG_KIND_TALK: {
			CGTK_add_message(chat_list, msg);
			break;
		} case MSG_KIND_JOIN: {
			GString* members = g_string_new(gtk_label_get_text(GTK_LABEL(port_label)));
			
			if (members->len > 0) {
				g_string_append(members, ", \0");
			}
			
			g_string_append(members, msg->who);
			
			gtk_label_set_text(GTK_LABEL(port_label), members->str);
			g_string_free(members, TRUE);
			break;
		} case MSG_KIND_LEAVE: {
			GString* members = g_string_new(gtk_label_get_text(GTK_LABEL(port_label)));
			
			const size_t who_len = strlen(msg->who);
			
			char* pos = members->str;
			
			while (pos) {
				pos = strstr(pos, msg->who);
				
				if (pos) {
					if (((pos == msg->who) || (*(pos - 1) == ' ')) &&
						((pos[who_len] == '\0') || ((pos[who_len] == ',') && (pos[who_len + 1] == ' ')))) {
						break;
					} else {
						pos++;
					}
				}
			}
			
			if (pos) {
				if (pos[who_len] == '\0') {
					if (pos == msg->who) {
						g_string_erase(members, 0, who_len);
					} else {
						g_string_erase(members, (ssize_t) (pos - msg->who) - 2, who_len + 2);
					}
				} else {
					g_string_erase(members, (ssize_t) (pos - msg->who), who_len + 2);
				}
				
				gtk_label_set_text(GTK_LABEL(port_label), members->str);
			}
			
			g_string_free(members, TRUE);
			break;
		} case MSG_KIND_INFO: {
			GString* members = g_string_new("\0");
			const char** part = msg->participants;
			
			while (*part) {
				if (members->len > 0) {
					g_string_append(members, ", \0");
				}
				
				g_string_append(members, *part);
				part++;
			}
			
			gtk_label_set_text(GTK_LABEL(port_label), members->str);
			g_string_free(members, TRUE);
			break;
		} default: {
			break;
		}
	}
}
