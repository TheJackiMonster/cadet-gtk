//
// Created by thejackimonster on 21.06.20.
//

static void CGTK_new_group_cancel(GtkWidget* cancel_button, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	gtk_widget_destroy(gui->new_group.dialog);
}

static void CGTK_new_group_confirm(GtkWidget* confirm_button, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	gboolean host_group = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui->new_group.host_check));
	
	const char* identity;
	
	if (host_group) {
		identity = gui->attributes.identity;
	} else {
		identity = CGTK_get_entry_text(gui->new_group.identity_entry);
	}
	
	const char* port = CGTK_get_entry_text(gui->new_group.port_entry);
	const char* name = CGTK_get_entry_text(gui->new_group.name_entry);
	
	cgtk_chat_t* chat = gui->callbacks.select_chat(identity, port);
	chat->is_group = TRUE;
	
	gui->callbacks.set_name(identity, port, name);
	
	if (host_group) {
		gui->callbacks.open_group(port);
	} else {
		CGTK_open_contact(gui, identity, port);
	}
	
	msg_t msg = {};
	msg.kind = MSG_KIND_JOIN;
	
	gui->callbacks.send_message(identity, port, &msg);
	
	gtk_widget_destroy(gui->new_group.dialog);
}

static void CGTK_new_group_id_changed(GtkEditable* id_editable, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	gtk_entry_set_placeholder_text(GTK_ENTRY(gui->new_group.name_entry), gtk_editable_get_chars(id_editable, 0, -1));
	
	gtk_widget_show_all(gui->new_group.name_entry);
}

static void CGTK_new_group_destroy(GtkWidget* dialog, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	memset(&(gui->new_group), 0, sizeof(gui->new_group));
}

static void CGTK_new_group_dialog(GtkWidget* add_button, gpointer user_data) {
	cgtk_gui_t *gui = (cgtk_gui_t *) user_data;
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gui->contacts.add_button), FALSE);
	
#ifdef HANDY_USE_ZERO_API
	if (gui->main.window) {
		gui->new_group.dialog = hdy_dialog_new(GTK_WINDOW(gui->main.window));
	} else {
		return;
	}
#else
	gui->new_group.dialog = gtk_dialog_new();
#endif
	
	gtk_window_set_title(GTK_WINDOW(gui->new_group.dialog), "Add group\0");
	gtk_widget_set_size_request(gui->new_group.dialog, 320, 0);
	
	GtkWidget* header_bar = gtk_dialog_get_header_bar(GTK_DIALOG(gui->new_group.dialog));
	
	GtkWidget* search_button = gtk_button_new_from_icon_name("system-search-symbolic\0", GTK_ICON_SIZE_MENU);
	
	gtk_header_bar_pack_end(GTK_HEADER_BAR(header_bar), search_button);
	
	GtkWidget* main_box = gtk_dialog_get_content_area(GTK_DIALOG(gui->new_group.dialog));
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
	
	GtkWidget* avatar = gtk_image_new_from_icon_name("system-users-symbolic\0", GTK_ICON_SIZE_DIALOG);
	gtk_widget_set_margin_bottom(avatar, 4);
	gtk_widget_set_margin_start(avatar, 8);
	gtk_widget_set_margin_top(avatar, 4);
	gtk_widget_set_margin_end(avatar, 8);
	
	gtk_container_add(GTK_CONTAINER(main_box), avatar);
	
	GtkWidget* id_label = gtk_label_new("ID\0");
	gtk_widget_set_hexpand(id_label, TRUE);
	
	GtkWidget* port_label = gtk_label_new("Port\0");
	gtk_widget_set_hexpand(port_label, TRUE);
	
	GtkWidget* name_label = gtk_label_new("Name\0");
	gtk_widget_set_hexpand(name_label, TRUE);
	
	gui->new_group.host_check = gtk_check_button_new_with_label("Create new group\0");
	gtk_widget_set_hexpand( gui->new_group.host_check, TRUE);
	
	gui->new_group.identity_entry = gtk_entry_new();
	gtk_widget_set_hexpand(gui->new_group.identity_entry, TRUE);
	
	gui->new_group.name_entry = gtk_entry_new();
	gtk_widget_set_hexpand(gui->new_group.name_entry, TRUE);
	
	gui->new_group.port_entry = gtk_entry_new();
	gtk_widget_set_hexpand(gui->new_group.port_entry, TRUE);
	
	gtk_grid_attach(GTK_GRID(grid), id_label, 0, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), name_label, 0, 2, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), port_label, 0, 3, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), gui->new_group.host_check, 1, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), gui->new_group.identity_entry, 1, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), gui->new_group.name_entry, 1, 2, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), gui->new_group.port_entry, 1, 3, 1, 1);
	
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
	
	g_signal_connect(gui->new_group.dialog, "destroy\0", G_CALLBACK(CGTK_new_group_destroy), gui);
	
	g_signal_connect(search_button, "clicked\0", G_CALLBACK(CGTK_id_search_dialog), gui);
	
	g_signal_connect(gui->new_group.identity_entry, "changed\0", G_CALLBACK(CGTK_new_group_id_changed), gui);
	
	g_signal_connect(cancel_button, "clicked\0", G_CALLBACK(CGTK_new_group_cancel), gui);
	g_signal_connect(confirm_button, "clicked\0", G_CALLBACK(CGTK_new_group_confirm), gui);
	
	g_object_bind_property(
			gui->new_group.host_check,"active\0",
			gui->new_group.identity_entry, "sensitive\0",
			G_BINDING_INVERT_BOOLEAN
	);
	
	gtk_widget_show_all(gui->new_group.dialog);
}