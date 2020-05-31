//
// Created by thejackimonster on 04.05.20.
//

static void CGTK_new_contact_cancel(GtkWidget* cancel_button, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	gtk_widget_destroy(gui->new_contact.dialog);
}

static void CGTK_new_contact_confirm(GtkWidget* confirm_button, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	const char* identity = CGTK_get_entry_text(gui->new_contact.identity_entry);
	const char* port = CGTK_get_entry_text(gui->new_contact.port_entry);
	const char* name = CGTK_get_entry_text(gui->new_contact.name_entry);
	gboolean is_group = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui->new_contact.group_check));
	
	cgtk_chat_t* chat = gui->callbacks.select_chat(identity, port);
	chat->is_group = is_group;
	
	gui->callbacks.set_name(identity, port, name);
	
	if ((is_group) && (strcmp(identity, gui->attributes.identity) == 0)) {
		gui->callbacks.open_group(port);
	} else {
		CGTK_open_contact(gui, identity, port);
	}
	
	if (is_group) {
		msg_t msg = {};
		msg.kind = MSG_KIND_JOIN;
		
		gui->callbacks.send_message(identity, port, &msg);
	}
	
	gtk_widget_destroy(gui->new_contact.dialog);
}

static void CGTK_new_contact_id_changed(GtkEditable* id_editable, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	gtk_entry_set_placeholder_text(GTK_ENTRY(gui->new_contact.name_entry), gtk_editable_get_chars(id_editable, 0, -1));
	
	gtk_widget_show_all(gui->new_contact.name_entry);
}

static void CGTK_new_contact_destroy(GtkWidget* dialog, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	memset(&(gui->new_contact), 0, sizeof(gui->new_contact));
}

static void CGTK_new_contact_dialog(GtkWidget* add_button, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;

#ifdef HANDY_USE_ZERO_API
	gui->new_contact.dialog = hdy_dialog_new(GTK_WINDOW(gui->main.window));
#else
	gui->new_contact.dialog = gtk_dialog_new();
#endif
	
	gtk_window_set_title(GTK_WINDOW(gui->new_contact.dialog), "Add contact\0");
	gtk_widget_set_size_request(gui->new_contact.dialog, 320, 0);
	
	GtkWidget* header_bar = gtk_dialog_get_header_bar(GTK_DIALOG(gui->new_contact.dialog));
	
	GtkWidget* search_button = gtk_button_new_from_icon_name("system-search-symbolic\0", GTK_ICON_SIZE_MENU);
	
	gtk_header_bar_pack_end(GTK_HEADER_BAR(header_bar), search_button);
	
	GtkWidget* main_box = gtk_dialog_get_content_area(GTK_DIALOG(gui->new_contact.dialog));
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
	
	GtkWidget* name_label = gtk_label_new("Name\0");
	gtk_widget_set_hexpand(name_label, TRUE);
	
	gui->new_contact.identity_entry = gtk_entry_new();
	gtk_widget_set_hexpand(gui->new_contact.identity_entry, TRUE);
	
	gui->new_contact.port_entry = gtk_entry_new();
	gtk_widget_set_hexpand(gui->new_contact.port_entry, TRUE);
	
	gui->new_contact.name_entry = gtk_entry_new();
	gtk_widget_set_hexpand(gui->new_contact.name_entry, TRUE);
	
	gui->new_contact.group_check = gtk_check_button_new_with_label("Join a groupchat?\0");
	gtk_widget_set_hexpand( gui->new_contact.group_check, TRUE);
	
	gtk_grid_attach(GTK_GRID(grid), id_label, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), port_label, 0, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), name_label, 0, 2, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), gui->new_contact.identity_entry, 1, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), gui->new_contact.port_entry, 1, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), gui->new_contact.name_entry, 1, 2, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), gui->new_contact.group_check, 1, 3, 1, 1);
	
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
	
	g_signal_connect(gui->new_contact.dialog, "destroy\0", G_CALLBACK(CGTK_new_contact_destroy), gui);
	
	g_signal_connect(search_button, "clicked\0", G_CALLBACK(CGTK_id_search_dialog), gui);
	
	g_signal_connect(gui->new_contact.identity_entry, "changed\0", G_CALLBACK(CGTK_new_contact_id_changed), gui);
	
	g_signal_connect(cancel_button, "clicked\0", G_CALLBACK(CGTK_new_contact_cancel), gui);
	g_signal_connect(confirm_button, "clicked\0", G_CALLBACK(CGTK_new_contact_confirm), gui);
	
	gtk_widget_show_all(gui->new_contact.dialog);
}
