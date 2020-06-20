//
// Created by thejackimonster on 20.06.20.
//

static void CGTK_view_files_cancel(GtkWidget* cancel_button, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	gtk_widget_destroy(gui->view_files.dialog);
}

static void CGTK_view_files_confirm(GtkWidget* confirm_button, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	gtk_widget_destroy(gui->view_files.dialog);
}

static void CGTK_view_files_destroy(GtkWidget* dialog, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	memset(&(gui->view_files), 0, sizeof(gui->view_files));
}

static void CGTK_view_files_dialog(GtkWidget* files_button, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	GString* name = g_string_new(gtk_stack_get_visible_child_name(GTK_STACK(gui->chat.stack)));
	
	const char* identity = name->str;
	const char* port = "\0";
	
	uint index = CGTK_split_name(name, &identity, &port);
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gui->chat.options_button), FALSE);
	
#ifdef HANDY_USE_ZERO_API
	gui->view_files.dialog = hdy_dialog_new(GTK_WINDOW(gui->main.window));
#else
	gui->view_files.dialog = gtk_dialog_new();
#endif
	
	gtk_window_set_title(GTK_WINDOW(gui->view_files.dialog), "View Files\0");
	gtk_widget_set_size_request(gui->view_files.dialog, 320, 0);
	
	GtkWidget* main_box = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(gui->view_files.dialog))->data);
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
	
	GtkWidget* upload_list = gtk_list_box_new();
	GtkWidget* download_list = gtk_list_box_new();
	
	const GList* files = g_hash_table_get_keys(chat->files);
	
	while (files) {
		GString* path_string = (GString*) files->data;
		
		//GtkWidget* file_box = GTK_WIDGET(g_hash_table_lookup(chat->files, files->data));
		
		const cgtk_file_t* file = CGTK_get_file(gui, path_string->str);
		
		HdyActionRow* entry = hdy_action_row_new();
		
		hdy_action_row_set_title(entry, file->name);
		hdy_action_row_set_subtitle(entry, file->hash);
		hdy_action_row_set_icon_name(entry, "text-x-generic-symbolic\0");
		
		gtk_container_add(GTK_CONTAINER(files_list), GTK_WIDGET(entry));
		
		gtk_widget_show_all(GTK_WIDGET(entry));
		
		files = files->next;
	}
	
	if (name->str[index] == '\0') {
		name->str[index] = '_';
	}
	
	g_string_free(name, TRUE);
	
	gtk_grid_attach(GTK_GRID(grid), files_list, 0, 0, 1, 1);
	
	GtkWidget* cancel_button = gtk_button_new_with_label("Cancel\0");
	GtkWidget* confirm_button = gtk_button_new_with_label("Confirm\0");
	
	gtk_container_add(GTK_CONTAINER(main_box), grid);
	gtk_container_add(GTK_CONTAINER(main_box), button_box);
	
	gtk_box_set_child_packing(GTK_BOX(main_box), grid, TRUE, TRUE, 2, GTK_PACK_START);
	gtk_box_set_child_packing(GTK_BOX(main_box), button_box, FALSE, FALSE, 2, GTK_PACK_END);
	
	gtk_container_add(GTK_CONTAINER(button_box), cancel_button);
	gtk_container_add(GTK_CONTAINER(button_box), confirm_button);
	
	g_signal_connect(gui->view_files.dialog, "destroy\0", G_CALLBACK(CGTK_view_files_destroy), gui);
	
	g_signal_connect(cancel_button, "clicked\0", G_CALLBACK(CGTK_view_files_cancel), gui);
	g_signal_connect(confirm_button, "clicked\0", G_CALLBACK(CGTK_view_files_confirm), gui);
	
	gtk_widget_show_all(gui->view_files.dialog);
}
