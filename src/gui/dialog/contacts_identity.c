//
// Created by thejackimonster on 04.05.20.
//

#include "../util.h"

static void CGTK_identity_cancel(GtkWidget* cancel_button, gpointer user_data) {
	cgtk_gui_t *gui = (cgtk_gui_t *) user_data;
	
	gtk_widget_destroy(gui->identity.dialog);
}

static void CGTK_identity_confirm(GtkWidget* confirm_button, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;

	const char* visibility_id = gtk_combo_box_get_active_id(GTK_COMBO_BOX(gui->identity.visibility_combobox));
	
	if (strcmp(visibility_id, CGTK_VISIBILITY_PUBLIC_ID) == 0) {
		gui->config.visibility = CGTK_VISIBILITY_PUBLIC;
	} else
	if (strcmp(visibility_id, CGTK_VISIBILITY_PRIVATE_ID) == 0) {
		gui->config.visibility = CGTK_VISIBILITY_PRIVATE;
	} else
	if (strcmp(visibility_id, CGTK_VISIBILITY_CAT_ID) == 0) {
		gui->config.visibility = CGTK_VISIBILITY_CAT;
	}
	
	strncpy(gui->config.port, CGTK_get_entry_text(gui->identity.port_entry), CGTK_PORT_BUFFER_SIZE - 1);
	gui->config.port[CGTK_PORT_BUFFER_SIZE - 1] = '\0';
	
	const char* name = CGTK_get_entry_text(gui->identity.name_entry);
	const char* mail = CGTK_get_entry_text(gui->identity.mail_entry);
	const char* phone = CGTK_get_entry_text(gui->identity.phone_entry);
	
	GString* regex = NULL;
	
	if (gtk_entry_get_text_length(GTK_ENTRY(gui->identity.name_entry)) > 0) {
		regex = CGTK_regex_append_escaped(regex, name);
	}
	
	if (gtk_entry_get_text_length(GTK_ENTRY(gui->identity.mail_entry)) > 0) {
		regex = CGTK_regex_append_escaped(regex, mail);
	}
	
	if (gtk_entry_get_text_length(GTK_ENTRY(gui->identity.phone_entry)) > 0) {
		regex = CGTK_regex_append_escaped(regex, phone);
	}
	
	if (regex) {
		g_string_append_c(regex, '\0');
	}
	
	strncpy(gui->config.nick, name, CGTK_NAME_BUFFER_SIZE);
	gui->config.nick[CGTK_NAME_BUFFER_SIZE - 1] = '\0';
	
	strncpy(gui->config.email, mail, CGTK_NAME_BUFFER_SIZE);
	gui->config.email[CGTK_NAME_BUFFER_SIZE - 1] = '\0';
	
	strncpy(gui->config.phone, phone, CGTK_NAME_BUFFER_SIZE);
	gui->config.phone[CGTK_NAME_BUFFER_SIZE - 1] = '\0';
	
	gui->callbacks.update_host(regex? regex->str : NULL);
	
	if (regex) {
		g_string_free(regex, TRUE);
	}
	
	gtk_widget_destroy(gui->identity.dialog);
}

static void CGTK_identity_destroy(GtkWidget* dialog, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	memset(&(gui->identity), 0, sizeof(gui->identity));
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
	
	GtkWidget* main_box = gtk_dialog_get_content_area(GTK_DIALOG(gui->identity.dialog));
	gtk_box_set_spacing(GTK_BOX(main_box), 2);
	gtk_widget_set_margin_start(main_box, 4);
	gtk_widget_set_margin_bottom(main_box, 4);
	gtk_widget_set_margin_end(main_box, 4);
	gtk_widget_set_margin_top(main_box, 4);
	gtk_widget_set_vexpand(main_box, TRUE);
	
	GtkWidget* button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(button_box), GTK_BUTTONBOX_END);
	gtk_box_set_spacing(GTK_BOX(button_box), 2);

	// TODO: Maybe put identity as QR code in here (backside of the avatar which can be shown via swipe)?
	
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
	
	GtkWidget* grid = gtk_grid_new();
	gtk_grid_set_column_homogeneous(GTK_GRID(grid), FALSE);
	gtk_grid_set_column_spacing(GTK_GRID(grid), 4);
	gtk_grid_set_row_homogeneous(GTK_GRID(grid), FALSE);
	gtk_grid_set_row_spacing(GTK_GRID(grid), 4);
	
	GtkWidget* advanced_expand = gtk_expander_new("Advanced\0");
	gtk_expander_set_expanded(GTK_EXPANDER(advanced_expand), FALSE);
	
	GtkWidget* advanced_grid = gtk_grid_new();
	gtk_grid_set_column_homogeneous(GTK_GRID(advanced_grid), FALSE);
	gtk_grid_set_column_spacing(GTK_GRID(advanced_grid), 4);
	gtk_grid_set_row_homogeneous(GTK_GRID(advanced_grid), FALSE);
	gtk_grid_set_row_spacing(GTK_GRID(advanced_grid), 4);
	
	gui->identity.label = gtk_label_new(gui->attributes.identity);
	gtk_label_set_line_wrap_mode(GTK_LABEL(gui->identity.label), PANGO_WRAP_CHAR);
	gtk_label_set_line_wrap(GTK_LABEL(gui->identity.label), TRUE);
	gtk_label_set_selectable(GTK_LABEL(gui->identity.label), TRUE);
	
	GtkWidget* name_label = gtk_label_new("Name\0");
	gtk_widget_set_hexpand(name_label, TRUE);
	
	GtkWidget* mail_label = gtk_label_new("Email\0");
	gtk_widget_set_hexpand(name_label, TRUE);
	
	GtkWidget* phone_label = gtk_label_new("Phone\0");
	gtk_widget_set_hexpand(phone_label, TRUE);
	
	gui->identity.name_entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(gui->identity.name_entry), gui->config.nick);
	gtk_entry_set_input_purpose(GTK_ENTRY(gui->identity.name_entry), GTK_INPUT_PURPOSE_NAME);
	gtk_widget_set_hexpand(gui->identity.name_entry, TRUE);
	
	gui->identity.mail_entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(gui->identity.mail_entry), gui->config.email);
	gtk_entry_set_input_purpose(GTK_ENTRY(gui->identity.mail_entry), GTK_INPUT_PURPOSE_EMAIL);
	gtk_widget_set_hexpand(gui->identity.mail_entry, TRUE);
	
	gui->identity.phone_entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(gui->identity.phone_entry), gui->config.phone);
	gtk_entry_set_input_purpose(GTK_ENTRY(gui->identity.phone_entry), GTK_INPUT_PURPOSE_PHONE);
	gtk_widget_set_hexpand(gui->identity.phone_entry, TRUE);
	
	gtk_grid_attach(GTK_GRID(grid), gui->identity.label, 0, 0, 2, 1);
	gtk_grid_attach(GTK_GRID(grid), name_label, 0, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), mail_label, 0, 2, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), phone_label, 0, 3, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), gui->identity.name_entry, 1, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), gui->identity.mail_entry, 1, 2, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), gui->identity.phone_entry, 1, 3, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), advanced_expand, 0, 4, 2, 1);
	
	GtkWidget* visibility_label = gtk_label_new("Visibility\0");
	gtk_widget_set_hexpand(visibility_label, TRUE);
	
	GtkWidget* port_label = gtk_label_new("Port\0");
	gtk_widget_set_hexpand(port_label, TRUE);
	
	const char* visibility_id = NULL;
	
	switch (gui->config.visibility) {
		case CGTK_VISIBILITY_PUBLIC:
			visibility_id = CGTK_VISIBILITY_PUBLIC_ID;
			break;
		case CGTK_VISIBILITY_PRIVATE:
			visibility_id = CGTK_VISIBILITY_PRIVATE_ID;
			break;
		case CGTK_VISIBILITY_CAT:
			visibility_id = CGTK_VISIBILITY_CAT_ID;
			break;
		default:
			break;
	}
	
	gui->identity.visibility_combobox = gtk_combo_box_text_new();
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(gui->identity.visibility_combobox), CGTK_VISIBILITY_PUBLIC_ID, "Public\0");
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(gui->identity.visibility_combobox), CGTK_VISIBILITY_PRIVATE_ID, "Hidden\0");
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(gui->identity.visibility_combobox), CGTK_VISIBILITY_CAT_ID, "Cat\0");
	
	if (visibility_id) {
		gtk_combo_box_set_active_id(GTK_COMBO_BOX(gui->identity.visibility_combobox), visibility_id);
	}
	
	gui->identity.port_entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(gui->identity.port_entry), gui->config.port);
	gtk_widget_set_hexpand(gui->identity.port_entry, TRUE);
	
	gtk_grid_attach(GTK_GRID(advanced_grid), visibility_label, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(advanced_grid), port_label, 0, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(advanced_grid), gui->identity.visibility_combobox, 1, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(advanced_grid), gui->identity.port_entry, 1, 1, 1, 1);
	
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
	
	g_signal_connect(gui->identity.dialog, "destroy\0", G_CALLBACK(CGTK_identity_destroy), gui);
	
	g_signal_connect(cancel_button, "clicked\0", G_CALLBACK(CGTK_identity_cancel), gui);
	g_signal_connect(confirm_button, "clicked\0", G_CALLBACK(CGTK_identity_confirm), gui);
	
	gtk_widget_show_all(gui->identity.dialog);
}
