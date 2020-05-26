//
// Created by thejackimonster on 04.05.20.
//

#include "../util.h"

static void CGTK_identity_port_confirm(GtkWidget* port_entry, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;

	strncpy(gui->attributes.port, CGTK_get_entry_text(gui->identity.port_entry), CGTK_PORT_BUFFER_SIZE - 1);
	gui->attributes.port[CGTK_PORT_BUFFER_SIZE - 1] = '\0';
	
	GString* regex = NULL;
	
	if (gtk_entry_get_text_length(GTK_ENTRY(gui->identity.name_entry)) > 0) {
		const char * name = gtk_entry_get_text(GTK_ENTRY(gui->identity.name_entry));
		
		regex = CGTK_regex_append_escaped(regex, name);
		
		gui->callbacks.set_name(gui->attributes.identity, gui->attributes.port, name);
	}
	
	if (gtk_entry_get_text_length(GTK_ENTRY(gui->identity.mail_entry)) > 0) {
		regex = CGTK_regex_append_escaped(regex, gtk_entry_get_text(GTK_ENTRY(gui->identity.mail_entry)));
	}
	
	if (gtk_entry_get_text_length(GTK_ENTRY(gui->identity.phone_entry)) > 0) {
		regex = CGTK_regex_append_escaped(regex, gtk_entry_get_text(GTK_ENTRY(gui->identity.phone_entry)));
	}
	
	if (regex) {
		g_string_append_c(regex, '\0');
	}
	
	gui->callbacks.update_host(gui->attributes.port, regex? regex->str : NULL);
	
	if (regex) {
		g_string_free(regex, TRUE);
	}
	
	gtk_widget_destroy(gui->identity.dialog);
}

static void CGTK_identity_dialog(GtkWidget* id_button, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;

#ifdef HANDY_USE_ZERO_API
	gui->identity.dialog = hdy_dialog_new(GTK_WINDOW(gui->main.window));
#else
	gui->identity.dialog = gtk_dialog_new();
#endif
	
	gtk_window_set_title(GTK_WINDOW(gui->identity.dialog), "Identity\0");
	gtk_widget_set_size_request(gui->identity.dialog, 300, 0);
	
	GtkWidget* main_box = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(gui->identity.dialog))->data);
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
	
	gui->identity.label = gtk_label_new(gui->attributes.identity);
	gtk_label_set_line_wrap_mode(GTK_LABEL(gui->identity.label), PANGO_WRAP_CHAR);
	gtk_label_set_line_wrap(GTK_LABEL(gui->identity.label), TRUE);
	gtk_label_set_selectable(GTK_LABEL(gui->identity.label), TRUE);
	
	gui->identity.port_entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(gui->identity.port_entry), gui->attributes.port);
	
	gui->identity.name_entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(gui->identity.name_entry), gui->callbacks.get_name(gui->attributes.identity, gui->attributes.port));
	gtk_entry_set_input_purpose(GTK_ENTRY(gui->identity.name_entry), GTK_INPUT_PURPOSE_NAME);
	
	gui->identity.mail_entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(gui->identity.mail_entry), "\0");
	gtk_entry_set_input_purpose(GTK_ENTRY(gui->identity.mail_entry), GTK_INPUT_PURPOSE_EMAIL);
	
	gui->identity.phone_entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(gui->identity.phone_entry), "\0");
	gtk_entry_set_input_purpose(GTK_ENTRY(gui->identity.phone_entry), GTK_INPUT_PURPOSE_PHONE);
	
	gtk_container_add(GTK_CONTAINER(main_box), gui->identity.label);
	gtk_container_add(GTK_CONTAINER(main_box), gui->identity.port_entry);
	gtk_container_add(GTK_CONTAINER(main_box), gui->identity.name_entry);
	gtk_container_add(GTK_CONTAINER(main_box), gui->identity.mail_entry);
	gtk_container_add(GTK_CONTAINER(main_box), gui->identity.phone_entry);
	
	g_signal_connect(gui->identity.port_entry, "activate\0", G_CALLBACK(CGTK_identity_port_confirm), gui);
	
	gtk_widget_show_all(gui->identity.dialog);
}
