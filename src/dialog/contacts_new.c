//
// Created by thejackimonster on 04.05.20.
//

static void CGTK_search_contact_changed(GtkEditable* name_editable, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	if (gui->search_list) {
		GList* children = gtk_container_get_children(GTK_CONTAINER(gui->search_list));
		
		while ((children) && (children->data)) {
			gtk_container_remove(GTK_CONTAINER(gui->search_list), children->data);
			
			children = children->next;
		}
	}
	
	gui->callbacks.search_by_name(gtk_editable_get_chars(name_editable, 0, -1));
}

static void CGTK_select_found_contact(GtkListBox* box, GtkListBoxRow* row, gpointer user_data) {
	GtkWidget* search_button = GTK_WIDGET(user_data);
	
	GtkWidget* search_dialog = gtk_widget_get_toplevel(GTK_WIDGET(box));
	GtkWidget* contact_dialog = gtk_widget_get_toplevel(search_button);
	
	const char* identity = gtk_widget_get_name(GTK_WIDGET(row));
	const char* name = hdy_action_row_get_title(HDY_ACTION_ROW(row));
	
	GtkWidget* main_box = gtk_dialog_get_content_area(GTK_DIALOG(contact_dialog));
	GtkWidget* grid = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(main_box))->data);
	
	GtkWidget* id_entry = gtk_grid_get_child_at(GTK_GRID(grid), 1, 0);
	GtkWidget* name_entry = gtk_grid_get_child_at(GTK_GRID(grid), 1, 2);
	
	gtk_entry_set_text(GTK_ENTRY(id_entry), identity);
	gtk_entry_set_text(GTK_ENTRY(name_entry), name);
	
	gtk_widget_destroy(search_dialog);
}

static void CGTK_search_contact_dialog(GtkWidget* search_button, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;

	GtkWidget* contact_dialog = gtk_widget_get_toplevel(search_button);
	
#ifdef HANDY_USE_ZERO_API
	GtkWidget* dialog = hdy_dialog_new(GTK_WINDOW(contact_dialog));
#else
	GtkWidget* dialog = gtk_dialog_new();
#endif
	
	gtk_window_set_title(GTK_WINDOW(dialog), "Search contact\0");
	gtk_widget_set_size_request(dialog, 320, 0);
	
	GtkWidget* search_box = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	gtk_box_set_spacing(GTK_BOX(search_box), 2);
	gtk_widget_set_margin_start(search_box, 4);
	gtk_widget_set_margin_bottom(search_box, 4);
	gtk_widget_set_margin_end(search_box, 4);
	gtk_widget_set_margin_top(search_box, 4);
	gtk_widget_set_vexpand(search_box, TRUE);
	
	gui->search_entry = gtk_entry_new();
	
	gtk_container_add(GTK_CONTAINER(search_box), gui->search_entry);
	
	gui->search_list = gtk_list_box_new();
	
	gtk_list_box_set_selection_mode(GTK_LIST_BOX(gui->search_list), GTK_SELECTION_BROWSE);
	gtk_widget_set_size_request(gui->search_list, 320, 0);
	gtk_widget_set_hexpand(gui->search_list, FALSE);
	gtk_widget_set_vexpand(gui->search_list, TRUE);
	
	GtkWidget* viewport = gtk_viewport_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(viewport), gui->search_list);
	
	GtkWidget* scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(
			GTK_SCROLLED_WINDOW(scrolled),
			GTK_POLICY_NEVER,
			GTK_POLICY_AUTOMATIC
	);
	
	gtk_container_add(GTK_CONTAINER(scrolled), viewport);
	gtk_container_add(GTK_CONTAINER(search_box), scrolled);
	
	g_signal_connect(gui->search_entry, "changed\0", G_CALLBACK(CGTK_search_contact_changed), gui);
	
	g_signal_connect(gui->search_list, "row-activated\0", G_CALLBACK(CGTK_select_found_contact), search_button);
	
	gtk_widget_show_all(dialog);
}

static void CGTK_add_contact_entry(GtkWidget* confirm_button, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	GtkWidget* dialog = gtk_widget_get_toplevel(confirm_button);
	
	GtkWidget* main_box = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	GtkWidget* grid = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(main_box))->data);
	
	GtkWidget* id_entry = gtk_grid_get_child_at(GTK_GRID(grid), 1, 0);
	GtkWidget* port_entry = gtk_grid_get_child_at(GTK_GRID(grid), 1, 1);
	GtkWidget* name_entry = gtk_grid_get_child_at(GTK_GRID(grid), 1, 2);
	GtkWidget* group_check = gtk_grid_get_child_at(GTK_GRID(grid), 1, 3);
	
	if (gui) {
		contact_type_t type;
		
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(group_check))) {
			type = CGTK_CONTACT_GROUP;
		} else {
			type = CGTK_CONTACT_PERSON;
		}
		
		const char* identity = gtk_entry_get_text(GTK_ENTRY(id_entry));
		const char* port = gtk_entry_get_text(GTK_ENTRY(port_entry));
		const char* name;
		
		if (gtk_entry_get_text_length(GTK_ENTRY(name_entry)) > 0) {
			name = gtk_entry_get_text(GTK_ENTRY(name_entry));
		} else {
			name = gtk_entry_get_placeholder_text(GTK_ENTRY(name_entry));
		}
		
		if ((type == CGTK_CONTACT_GROUP) && (strcmp(identity, gui->identity) == 0)) {
			gui->callbacks.open_group(port);
		} else {
			CGTK_open_contact(gui, identity, port, name, type);
		}
		
		if (type == CGTK_CONTACT_GROUP) {
			msg_t msg = {};
			msg.kind = MSG_KIND_JOIN;
			
			gui->callbacks.send_message(identity, port, &msg);
		}
	}
	
	gtk_widget_destroy(dialog);
}

static void CGTK_new_contact_id_changed(GtkEditable* id_editable, gpointer user_data) {
	GtkWidget* name_entry = GTK_WIDGET(user_data);
	
	gtk_entry_set_placeholder_text(GTK_ENTRY(name_entry), gtk_editable_get_chars(id_editable, 0, -1));
	
	gtk_widget_show_all(name_entry);
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
	
	GtkWidget* header_bar = gtk_dialog_get_header_bar(GTK_DIALOG(dialog));
	
	GtkWidget* search_button = gtk_button_new_from_icon_name("system-search-symbolic\0", GTK_ICON_SIZE_MENU);
	
	gtk_header_bar_pack_end(GTK_HEADER_BAR(header_bar), search_button);
	
	GtkWidget* main_box = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
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
	
	GtkWidget* id_entry = gtk_entry_new();
	gtk_widget_set_hexpand(id_entry, TRUE);
	
	GtkWidget* port_entry = gtk_entry_new();
	gtk_widget_set_hexpand(port_entry, TRUE);
	
	GtkWidget* name_entry = gtk_entry_new();
	gtk_widget_set_hexpand(name_entry, TRUE);
	
	GtkWidget* group_check = gtk_check_button_new_with_label("Join a groupchat?\0");
	gtk_widget_set_hexpand(group_check, TRUE);
	
	gtk_grid_attach(GTK_GRID(grid), id_label, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), port_label, 0, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), name_label, 0, 2, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), id_entry, 1, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), port_entry, 1, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), name_entry, 1, 2, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), group_check, 1, 3, 1, 1);
	
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
	
	g_signal_connect(search_button, "clicked\0", G_CALLBACK(CGTK_search_contact_dialog), gui);
	
	g_signal_connect(id_entry, "changed\0", G_CALLBACK(CGTK_new_contact_id_changed), name_entry);
	
	g_signal_connect(cancel_button, "clicked\0", G_CALLBACK(CGTK_add_contact_entry), NULL);
	g_signal_connect(confirm_button, "clicked\0", G_CALLBACK(CGTK_add_contact_entry), gui);
	
	gtk_widget_show_all(dialog);
}
