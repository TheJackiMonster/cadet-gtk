//
// Created by thejackimonster on 04.05.20.
//

static void CGTK_writeback_port(GtkWidget* id_entry, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	GtkWidget* dialog = gtk_widget_get_toplevel(id_entry);
	
	const char* port = gtk_entry_get_text(GTK_ENTRY(id_entry));
	size_t port_length = strlen(port);
	
	strncpy(gui->port, port, CGTK_PORT_BUFFER_SIZE - 1);
	gui->port[CGTK_PORT_BUFFER_SIZE - 1] = '\0';
	
	gui->callbacks.update_port();
	
	gtk_widget_destroy(dialog);
}

static void CGTK_open_identity(GtkWidget* id_button, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;

#ifdef HANDY_USE_ZERO_API
	GtkWidget* dialog = hdy_dialog_new(GTK_WINDOW(gui->app_window));
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
	
	GtkWidget* id_label = gtk_label_new(gui->identity);
	gtk_label_set_line_wrap_mode(GTK_LABEL(id_label), PANGO_WRAP_CHAR);
	gtk_label_set_line_wrap(GTK_LABEL(id_label), TRUE);
	gtk_label_set_selectable(GTK_LABEL(id_label), TRUE);
	
	GtkWidget* port_entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(port_entry), gui->port);
	
	gtk_container_add(GTK_CONTAINER(main_box), id_label);
	gtk_container_add(GTK_CONTAINER(main_box), port_entry);
	
	g_signal_connect(port_entry, "activate\0", G_CALLBACK(CGTK_writeback_port), gui);
	
	gtk_widget_show_all(dialog);
}
