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

static void CGTK_add_contact_entry(GtkWidget* confirm_button, gpointer user_data) {
	GtkWidget* dialog = gtk_widget_get_toplevel(confirm_button);
	
	GtkWidget* main_box = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(dialog))->data);
	GtkWidget* grid = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(main_box))->data);
	
	GtkWidget* id_entry = gtk_grid_get_child_at(GTK_GRID(grid), 1, 0);
	GtkWidget* port_entry = gtk_grid_get_child_at(GTK_GRID(grid), 1, 1);
	GtkWidget* group_check = gtk_grid_get_child_at(GTK_GRID(grid), 1, 2);
	
	if (user_data) {
		contact_type_t type;
		
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(group_check))) {
			type = CGTK_CONTACT_GROUP;
		} else {
			type = CGTK_CONTACT_PERSON;
		}
		
		CGTK_open_contact(
				GTK_WIDGET(user_data),
				gtk_entry_get_text(GTK_ENTRY(id_entry)),
				gtk_entry_get_text(GTK_ENTRY(port_entry)),
				type
		);
	}
	
	gtk_widget_destroy(dialog);
}

static void CGTK_add_contact_dialog(GtkWidget* add_button, gpointer user_data) {
	GtkWidget* contacts_list = GTK_WIDGET(user_data);
	GtkWidget* window = gtk_widget_get_toplevel(contacts_list);
	
#ifdef HANDY_USE_ZERO_API
	GtkWidget* dialog = hdy_dialog_new(GTK_WINDOW(window));
#else
	GtkWidget* dialog = gtk_dialog_new();
#endif
	
	gtk_window_set_title(GTK_WINDOW(dialog), "Add contact\0");
	gtk_widget_set_size_request(dialog, 320, 0);
	
	GtkWidget* main_box = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(dialog))->data);
	gtk_box_set_spacing(GTK_BOX(main_box), 2);
	gtk_widget_set_margin_start(main_box, 4);
	gtk_widget_set_margin_bottom(main_box, 4);
	gtk_widget_set_margin_end(main_box, 4);
	gtk_widget_set_margin_top(main_box, 4);
	gtk_widget_set_vexpand(main_box, TRUE);
	
	GtkWidget* button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(button_box), GTK_BUTTONBOX_END);
	gtk_box_set_spacing(GTK_BOX(button_box), 2);
	
	GtkWidget* grid = gtk_grid_new();
	gtk_grid_set_column_homogeneous(GTK_GRID(grid), FALSE);
	gtk_grid_set_column_spacing(GTK_GRID(grid), 4);
	gtk_grid_set_row_homogeneous(GTK_GRID(grid), FALSE);
	gtk_grid_set_row_spacing(GTK_GRID(grid), 4);
	
	GtkWidget* id_label = gtk_label_new("ID\0");
	gtk_widget_set_hexpand(id_label, TRUE);
	
	GtkWidget* port_label = gtk_label_new("Port\0");
	gtk_widget_set_hexpand(port_label, TRUE);
	
	GtkWidget* id_entry = gtk_entry_new();
	gtk_widget_set_hexpand(id_entry, TRUE);
	
	GtkWidget* port_entry = gtk_entry_new();
	gtk_widget_set_hexpand(port_entry, TRUE);
	
	GtkWidget* group_check = gtk_check_button_new_with_label("Join a groupchat?\0");
	gtk_widget_set_hexpand(group_check, TRUE);
	
	gtk_grid_attach(GTK_GRID(grid), id_label, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), port_label, 0, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), id_entry, 1, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), port_entry, 1, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), group_check, 1, 2, 1, 1);
	
	GtkWidget* cancel_button = gtk_button_new_with_label("Cancel\0");
	GtkWidget* confirm_button = gtk_button_new_with_label("Confirm\0");
	
	gtk_container_add(GTK_CONTAINER(main_box), grid);
	gtk_container_add(GTK_CONTAINER(main_box), button_box);
	
	gtk_box_set_child_packing(GTK_BOX(main_box), grid, TRUE, TRUE, 2, GTK_PACK_START);
	gtk_box_set_child_packing(GTK_BOX(main_box), button_box, FALSE, FALSE, 2, GTK_PACK_END);
	
	gtk_container_add(GTK_CONTAINER(button_box), cancel_button);
	gtk_container_add(GTK_CONTAINER(button_box), confirm_button);
	
	gtk_box_set_child_packing(GTK_BOX(button_box), cancel_button, FALSE, FALSE, 2, GTK_PACK_START);
	gtk_box_set_child_packing(GTK_BOX(button_box), confirm_button, FALSE, FALSE, 2, GTK_PACK_START);
	
	g_signal_connect(cancel_button, "clicked\0", G_CALLBACK(CGTK_add_contact_entry), NULL);
	g_signal_connect(confirm_button, "clicked\0", G_CALLBACK(CGTK_add_contact_entry), user_data);
	
	gtk_widget_show_all(dialog);
}

void CGTK_init_contacts(GtkWidget* header, GtkWidget* content, GtkWidget* contacts_list, const handy_callbacks_t* callbacks) {
	gtk_header_bar_set_title(GTK_HEADER_BAR(header), "Contacts\0");
	gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header), TRUE);
	gtk_header_bar_set_has_subtitle(GTK_HEADER_BAR(header), FALSE);
	
	GtkWidget* add_button = gtk_button_new_from_icon_name("list-add\0", GTK_ICON_SIZE_MENU);
	
	gtk_container_add(GTK_CONTAINER(header), add_button);
	
	gtk_list_box_set_selection_mode(GTK_LIST_BOX(contacts_list), GTK_SELECTION_BROWSE);
	gtk_widget_set_size_request(contacts_list, 320, 0);
	gtk_widget_set_hexpand(contacts_list, FALSE);
	gtk_widget_set_vexpand(contacts_list, TRUE);
	
	gtk_container_add(GTK_CONTAINER(content), contacts_list);
	
	// TODO: load all contacts
	
	GtkSizeGroup* sizeGroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	gtk_size_group_add_widget(sizeGroup, header);
	gtk_size_group_add_widget(sizeGroup, content);
	
	g_signal_connect(add_button, "clicked\0", G_CALLBACK(CGTK_add_contact_dialog), contacts_list);
	g_signal_connect(contacts_list, "row-activated\0", G_CALLBACK(callbacks->activate_contact), content);
}

void CGTK_open_contact(GtkWidget* contacts_list, const char* identity, const char* port, contact_type_t type) {
	GList* list = gtk_container_get_children(GTK_CONTAINER(contacts_list));
	
	GString* name = g_string_new(identity);
	g_string_append_c(name, '_');
	g_string_append(name, port);
	
	while (list) {
		GtkWidget* row = GTK_WIDGET(list->data);
		
		if (strcmp(gtk_widget_get_name(row), name->str) == 0) {
			hdy_action_row_set_icon_name(HDY_ACTION_ROW(row), "user-available-symbolic\0");
			g_string_free(name, TRUE);
			return;
		}
		
		list = list->next;
	}
	
	HdyActionRow* contact = hdy_action_row_new();
	gtk_widget_set_name(GTK_WIDGET(contact), name->str);
	
	g_string_free(name, TRUE);
	
	name = g_string_new(port);
	
	if (type == CGTK_CONTACT_GROUP) {
		g_string_append(name, " (GROUP)\0");
	}
	
	hdy_action_row_set_title(contact, name->str);
	hdy_action_row_set_subtitle(contact, identity);
	hdy_action_row_set_icon_name(contact, "user-available-symbolic\0");
	
	g_string_free(name, TRUE);
	
	gtk_container_add(GTK_CONTAINER(contacts_list),  GTK_WIDGET(contact));
	
	gtk_widget_show_all(GTK_WIDGET(contact));
}

void CGTK_close_contact(GtkWidget* contacts_list, const char* identity, const char* port) {
	GList* list = gtk_container_get_children(GTK_CONTAINER(contacts_list));
	
	GString* name = g_string_new(identity);
	g_string_append_c(name, '_');
	g_string_append(name, port);
	
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
