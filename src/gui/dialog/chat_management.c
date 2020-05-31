//
// Created by thejackimonster on 04.05.20.
//

#include "../contacts.h"

static void CGTK_management_cancel(GtkWidget* cancel_button, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	gtk_widget_destroy(gui->management.dialog);
}

static void CGTK_management_confirm(GtkWidget* confirm_button, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	const char* destination = gtk_label_get_text(GTK_LABEL(gui->management.identity_label));
	const char* port = gtk_label_get_text(GTK_LABEL(gui->management.port_label));
	const char* name = CGTK_get_entry_text(gui->management.name_entry);
	
	gui->callbacks.set_name(destination, port, name);
	
	gtk_widget_destroy(gui->management.dialog);
}

static void CGTK_management_exit_chat(GtkWidget* exit_button, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;

	const char* destination = gtk_label_get_text(GTK_LABEL(gui->management.identity_label));
	const char* port = gtk_label_get_text(GTK_LABEL(gui->management.port_label));
	
	gui->callbacks.exit_chat(destination, port);
	
	CGTK_remove_contact(gui, destination, port);
	
	gtk_widget_destroy(gui->management.dialog);
	
	if (strcmp(hdy_leaflet_get_visible_child_name(HDY_LEAFLET(gui->main.leaflet)), "contacts\0") != 0) {
		hdy_leaflet_set_visible_child_name(HDY_LEAFLET(gui->main.leaflet), "contacts\0");
	}
}

static void CGTK_management_destroy(GtkWidget* dialog, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	memset(&(gui->management), 0, sizeof(gui->management));
}

static void CGTK_management_dialog(GtkWidget* manage_button, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	GString* name = g_string_new(gtk_stack_get_visible_child_name(GTK_STACK(gui->chat.stack)));
	
	const char* identity = name->str;
	const char* port = "\0";
	
	uint index = CGTK_split_name(name, &identity, &port);
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gui->chat.options_button), FALSE);
	
	#ifdef HANDY_USE_ZERO_API
	gui->management.dialog = hdy_dialog_new(GTK_WINDOW(gui->main.window));
	#else
	gui->management.dialog = gtk_dialog_new();
	#endif
	
	gtk_window_set_title(GTK_WINDOW(gui->management.dialog), "Manage Chat\0");
	gtk_widget_set_size_request(gui->management.dialog, 320, 0);
	
	GtkWidget* main_box = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(gui->management.dialog))->data);
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
	
	const cgtk_chat_t* chat = gui->callbacks.select_chat(identity, port);
	
	GtkWidget* members_list = gtk_list_box_new();
	const GList* members = chat->members;
	
	while (members) {
		const cgtk_member_t* member = (cgtk_member_t*) members->data;
		
		HdyActionRow* contact = hdy_action_row_new();
		
		hdy_action_row_set_title(contact, member->name);
		hdy_action_row_set_subtitle(contact, member->identity);
		hdy_action_row_set_icon_name(contact, "user-available-symbolic\0");
		
		gtk_container_add(GTK_CONTAINER(members_list), GTK_WIDGET(contact));
		
		gtk_widget_show_all(GTK_WIDGET(contact));
		
		members = members->next;
	}
	
	GtkWidget* port_label = gtk_label_new("Port\0");
	gtk_widget_set_hexpand(port_label, TRUE);
	
	GtkWidget* name_label = gtk_label_new("Name\0");
	gtk_widget_set_hexpand(name_label, TRUE);
	
	gui->management.identity_label = gtk_label_new(identity);
	gtk_label_set_line_wrap_mode(GTK_LABEL(gui->management.identity_label), PANGO_WRAP_CHAR);
	gtk_label_set_line_wrap(GTK_LABEL(gui->management.identity_label), TRUE);
	gtk_label_set_selectable(GTK_LABEL(gui->management.identity_label), TRUE);
	gtk_widget_set_hexpand(gui->management.identity_label, TRUE);
	
	gui->management.port_label = gtk_label_new(port);
	gtk_label_set_line_wrap_mode(GTK_LABEL(gui->management.port_label), PANGO_WRAP_CHAR);
	gtk_label_set_line_wrap(GTK_LABEL(gui->management.port_label), TRUE);
	gtk_label_set_selectable(GTK_LABEL(gui->management.port_label), TRUE);
	gtk_widget_set_hexpand(gui->management.port_label, TRUE);
	
	gui->management.name_entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(gui->management.name_entry), gui->callbacks.get_name(identity, port));
	gtk_widget_set_hexpand(gui->management.name_entry, TRUE);
	
	if (name->str[index] == '\0') {
		name->str[index] = '_';
	}
	
	g_string_free(name, TRUE);
	
	GtkWidget* exit_button = gtk_button_new_with_label("Exit Chat\0");
	
	gtk_grid_attach(GTK_GRID(grid), gui->management.identity_label, 0, 0, 2, 1);
	gtk_grid_attach(GTK_GRID(grid), members_list, 0, 1, 2, 1);
	gtk_grid_attach(GTK_GRID(grid), port_label, 0, 2, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), name_label, 0, 3, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), gui->management.port_label, 1, 2, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), gui->management.name_entry, 1, 3, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), exit_button, 1, 4, 1, 1);
	
	GtkWidget* cancel_button = gtk_button_new_with_label("Cancel\0");
	GtkWidget* confirm_button = gtk_button_new_with_label("Confirm\0");
	
	gtk_container_add(GTK_CONTAINER(main_box), grid);
	gtk_container_add(GTK_CONTAINER(main_box), button_box);
	
	gtk_box_set_child_packing(GTK_BOX(main_box), grid, TRUE, TRUE, 2, GTK_PACK_START);
	gtk_box_set_child_packing(GTK_BOX(main_box), button_box, FALSE, FALSE, 2, GTK_PACK_END);
	
	gtk_container_add(GTK_CONTAINER(button_box), cancel_button);
	gtk_container_add(GTK_CONTAINER(button_box), confirm_button);
	
	g_signal_connect(gui->management.dialog, "destroy\0", G_CALLBACK(CGTK_management_destroy), gui);
	
	g_signal_connect(exit_button, "clicked\0", G_CALLBACK(CGTK_management_exit_chat), gui);
	
	g_signal_connect(cancel_button, "clicked\0", G_CALLBACK(CGTK_management_cancel), gui);
	g_signal_connect(confirm_button, "clicked\0", G_CALLBACK(CGTK_management_confirm), gui);
	
	gtk_widget_show_all(gui->management.dialog);
}
