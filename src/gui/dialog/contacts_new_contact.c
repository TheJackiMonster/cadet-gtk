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
	
	cgtk_chat_t* chat = gui->callbacks.select_chat(identity, port);
	chat->is_group = FALSE;
	
	gui->callbacks.set_name(identity, port, name);
	
	CGTK_open_contact(gui, identity, port);
	
	gtk_widget_destroy(gui->new_contact.dialog);
}

static void CGTK_new_contact_id_changed(GtkEditable* id_editable, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	gtk_entry_set_placeholder_text(GTK_ENTRY(gui->new_contact.name_entry), gtk_editable_get_chars(id_editable, 0, -1));
	
	gtk_widget_show_all(gui->new_contact.name_entry);
}

static void CGTK_init_new_contact_dialog(cgtk_gui_t* gui, GtkWidget* main_box) {
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
	
	GtkWidget* advanced_expand = gtk_expander_new("Advanced\0");
	gtk_expander_set_expanded(GTK_EXPANDER(advanced_expand), FALSE);
	gtk_widget_set_margin_top(advanced_expand, 16);
	
	GtkWidget* advanced_grid = gtk_grid_new();
	gtk_grid_set_column_homogeneous(GTK_GRID(advanced_grid), FALSE);
	gtk_grid_set_column_spacing(GTK_GRID(advanced_grid), 4);
	gtk_grid_set_row_homogeneous(GTK_GRID(advanced_grid), FALSE);
	gtk_grid_set_row_spacing(GTK_GRID(advanced_grid), 4);
	gtk_widget_set_margin_top(advanced_grid, 6);
	
#ifndef HANDY_USE_ZERO_API
	char capitals [8];
	
	for (int i = 0; i < 4; i++) {
		capitals[i*2] = gui->attributes.identity[i];
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
	
	GtkWidget* id_label = gtk_label_new("ID\0");
	gtk_widget_set_hexpand(id_label, TRUE);
	
	GtkWidget* name_label = gtk_label_new("Name\0");
	gtk_widget_set_hexpand(name_label, TRUE);
	
	GtkWidget* port_label = gtk_label_new("Port\0");
	gtk_widget_set_hexpand(port_label, TRUE);
	
	gui->new_contact.identity_entry = gtk_entry_new();
	gtk_widget_set_hexpand(gui->new_contact.identity_entry, TRUE);
	
	gui->new_contact.name_entry = gtk_entry_new();
	gtk_widget_set_hexpand(gui->new_contact.name_entry, TRUE);
	
	gui->new_contact.port_entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(gui->new_contact.port_entry), gui->config.port);
	gtk_widget_set_hexpand(gui->new_contact.port_entry, TRUE);
	
	gtk_grid_attach(GTK_GRID(grid), id_label, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), name_label, 0, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), gui->new_contact.identity_entry, 1, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), gui->new_contact.name_entry, 1, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), advanced_expand, 0, 2, 2, 1);
	
	gtk_grid_attach(GTK_GRID(advanced_grid), port_label, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(advanced_grid), gui->new_contact.port_entry, 1, 0, 1, 1);
	
	GtkWidget* cancel_button = gtk_button_new_with_label("Cancel\0");
	GtkWidget* confirm_button = gtk_button_new_with_label("Confirm\0");
	
	gtk_container_add(GTK_CONTAINER(advanced_expand), advanced_grid);
	gtk_container_add(GTK_CONTAINER(main_box), grid);
	gtk_container_add(GTK_CONTAINER(main_box), button_box);
	
	gtk_box_set_child_packing(GTK_BOX(main_box), grid, TRUE, TRUE, 2, GTK_PACK_START);
	gtk_box_set_child_packing(GTK_BOX(main_box), button_box, FALSE, FALSE, 2, GTK_PACK_END);
	
	gtk_container_add(GTK_CONTAINER(button_box), cancel_button);
	gtk_container_add(GTK_CONTAINER(button_box), confirm_button);
	
	gtk_box_set_child_packing(GTK_BOX(button_box), cancel_button, FALSE, FALSE, 2, GTK_PACK_START);
	gtk_box_set_child_packing(GTK_BOX(button_box), confirm_button, FALSE, FALSE, 2, GTK_PACK_START);
	
	g_signal_connect(gui->new_contact.identity_entry, "changed\0", G_CALLBACK(CGTK_new_contact_id_changed), gui);
	
	g_signal_connect(cancel_button, "clicked\0", G_CALLBACK(CGTK_new_contact_cancel), gui);
	g_signal_connect(confirm_button, "clicked\0", G_CALLBACK(CGTK_new_contact_confirm), gui);
}
