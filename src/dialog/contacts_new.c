//
// Created by thejackimonster on 04.05.20.
//

static void CGTK_add_contact_entry(GtkWidget* confirm_button, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	GtkWidget* dialog = gtk_widget_get_toplevel(confirm_button);
	
	GtkWidget* main_box = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(dialog))->data);
	GtkWidget* grid = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(main_box))->data);
	
	GtkWidget* id_entry = gtk_grid_get_child_at(GTK_GRID(grid), 1, 0);
	GtkWidget* port_entry = gtk_grid_get_child_at(GTK_GRID(grid), 1, 1);
	GtkWidget* group_check = gtk_grid_get_child_at(GTK_GRID(grid), 1, 2);
	
	if (gui) {
		contact_type_t type;
		
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(group_check))) {
			type = CGTK_CONTACT_GROUP;
		} else {
			type = CGTK_CONTACT_PERSON;
		}
		
		CGTK_open_contact(
				gui,
				gtk_entry_get_text(GTK_ENTRY(id_entry)),
				gtk_entry_get_text(GTK_ENTRY(port_entry)),
				type
		);
	}
	
	gtk_widget_destroy(dialog);
}

static void CGTK_add_contact_dialog(GtkWidget* add_button, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;

#ifdef HANDY_USE_ZERO_API
	GtkWidget* dialog = hdy_dialog_new(GTK_WINDOW(gui->app_window));
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
	g_signal_connect(confirm_button, "clicked\0", G_CALLBACK(CGTK_add_contact_entry), gui);
	
	gtk_widget_show_all(dialog);
}
