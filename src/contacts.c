//
// Created by thejackimonster on 12.04.20.
//

#include "contacts.h"

#define HANDY_USE_UNSTABLE_API
#include <libhandy-0.0/handy.h>

void add_contact(GtkWidget* confirm_button, gpointer user_data) {
	GtkWidget* dialog = gtk_widget_get_toplevel(confirm_button);
	
	if (user_data) {
		GtkWidget* contacts_list = GTK_WIDGET(user_data);
		
		//
		
		
	}
	
	gtk_widget_destroy(dialog);
}

void add_contact_dialog(GtkWidget* add_button, gpointer user_data) {
	GtkWidget* contacts_list = GTK_WIDGET(user_data);
	GtkWidget* window = gtk_widget_get_toplevel(contacts_list);
	
	GtkWidget* dialog = hdy_dialog_new(GTK_WINDOW(window));
	gtk_window_set_title(GTK_WINDOW(dialog), "Add contact");
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
	
	GtkWidget* id_label = gtk_label_new("ID");
	gtk_widget_set_hexpand(id_label, TRUE);
	
	GtkWidget* port_label = gtk_label_new("Port");
	gtk_widget_set_hexpand(port_label, TRUE);
	
	GtkWidget* id_entry = gtk_entry_new();
	gtk_widget_set_hexpand(id_entry, TRUE);
	
	GtkWidget* port_entry = gtk_entry_new();
	gtk_widget_set_hexpand(port_entry, TRUE);
	
	gtk_grid_attach(GTK_GRID(grid), id_label, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), port_label, 0, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), id_entry, 1, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), port_entry, 1, 1, 1, 1);
	
	GtkWidget* cancel_button = gtk_button_new_with_label("Cancel");
	GtkWidget* confirm_button = gtk_button_new_with_label("Confirm");
	
	gtk_container_add(GTK_CONTAINER(main_box), grid);
	gtk_container_add(GTK_CONTAINER(main_box), button_box);
	
	gtk_box_set_child_packing(GTK_BOX(main_box), grid, TRUE, TRUE, 2, GTK_PACK_START);
	gtk_box_set_child_packing(GTK_BOX(main_box), button_box, FALSE, FALSE, 2, GTK_PACK_END);
	
	gtk_container_add(GTK_CONTAINER(button_box), cancel_button);
	gtk_container_add(GTK_CONTAINER(button_box), confirm_button);
	
	gtk_box_set_child_packing(GTK_BOX(button_box), cancel_button, FALSE, FALSE, 2, GTK_PACK_START);
	gtk_box_set_child_packing(GTK_BOX(button_box), confirm_button, FALSE, FALSE, 2, GTK_PACK_START);
	
	g_signal_connect(confirm_button, "clicked", G_CALLBACK(add_contact), contacts_list);
	g_signal_connect(cancel_button, "clicked", G_CALLBACK(add_contact), NULL);
	
	gtk_widget_show_all(dialog);
}

void cadet_gtk_init_contacts(GtkWidget* header, GtkWidget* content, GtkWidget* contacts_list) {
	gtk_header_bar_set_title(GTK_HEADER_BAR(header), "Contacts");
	gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header), TRUE);
	gtk_header_bar_set_has_subtitle(GTK_HEADER_BAR(header), FALSE);
	
	GtkWidget* add_button = gtk_button_new_from_icon_name("list-add", GTK_ICON_SIZE_MENU);
	
	gtk_container_add(GTK_CONTAINER(header), add_button);
	
	gtk_container_add(GTK_CONTAINER(content), contacts_list);
	
	//
	
	GtkSizeGroup* sizeGroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	gtk_size_group_add_widget(sizeGroup, header);
	gtk_size_group_add_widget(sizeGroup, content);
	
	g_signal_connect(add_button, "clicked", G_CALLBACK(add_contact_dialog), contacts_list);
}
