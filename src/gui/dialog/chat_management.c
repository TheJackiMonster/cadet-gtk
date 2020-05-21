//
// Created by thejackimonster on 04.05.20.
//

#include "../contacts.h"

static void CGTK_exit_chat(GtkWidget* exit_button, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	GString* name = g_string_new(gtk_stack_get_visible_child_name(GTK_STACK(gui->chat.stack)));
	
	const char* destination = name->str;
	const char* port = "\0";
	
	uint index = CGTK_split_name(name, &destination, &port);
	
	gui->callbacks.exit_chat(destination, port);
	
	CGTK_remove_contact(gui, destination, port);
	
	if (name->str[index] == '\0') {
		name->str[index] = '_';
	}
	
	g_string_free(name, TRUE);
	
	gtk_widget_destroy(gui->management.dialog);
	
	if (strcmp(hdy_leaflet_get_visible_child_name(HDY_LEAFLET(gui->main.leaflet)), "contacts\0") != 0) {
		hdy_leaflet_set_visible_child_name(HDY_LEAFLET(gui->main.leaflet), "contacts\0");
	}
}

static void CGTK_manage_chat_dialog(GtkWidget* manage_button, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
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
	
	GtkWidget* exit_button = gtk_button_new_with_label("Exit Chat\0");
	gtk_widget_set_halign(exit_button, GTK_ALIGN_END);
	
	gtk_container_add(GTK_CONTAINER(main_box), exit_button);
	
	gtk_box_set_child_packing(GTK_BOX(main_box), exit_button, FALSE, FALSE, 2, GTK_PACK_END);
	
	g_signal_connect(exit_button, "clicked\0", G_CALLBACK(CGTK_exit_chat), gui);
	
	gtk_widget_show_all(gui->management.dialog);
}
