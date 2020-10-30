//
// Created by thejackimonster on 21.05.20.
//

static void CGTK_id_search_changed(GtkEditable* name_editable, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	GList* children = gtk_container_get_children(GTK_CONTAINER(gui->id_search.list));
	
	while ((children) && (children->data)) {
		gtk_container_remove(GTK_CONTAINER(gui->id_search.list), children->data);
		
		children = children->next;
	}
	
	gui->callbacks.search_by_name(gtk_editable_get_chars(name_editable, 0, -1));
}

static void CGTK_id_search_select(GtkListBox* box, GtkListBoxRow* row, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	const char* identity = gtk_widget_get_name(GTK_WIDGET(row));
	const char* name = hdy_preferences_row_get_title(HDY_PREFERENCES_ROW(row));
	
	if (gui->new_contact.dialog) {
		gtk_entry_set_text(GTK_ENTRY(gui->new_contact.identity_entry), identity);
		gtk_entry_set_text(GTK_ENTRY(gui->new_contact.name_entry), name);
	} else
	if (gui->new_group.dialog) {
		gtk_entry_set_text(GTK_ENTRY(gui->new_group.identity_entry), identity);
		gtk_entry_set_text(GTK_ENTRY(gui->new_group.name_entry), name);
	}
	
	gtk_widget_destroy(gui->id_search.dialog);
}

void CGTK_id_search_entry_found(cgtk_gui_t* gui, const char* name, const char* identity) {
	GList *list = gtk_container_get_children(GTK_CONTAINER(gui->id_search.list));
	gboolean duplicate = FALSE;
	
	while (list) {
		GtkWidget *row = GTK_WIDGET(list->data);
		
		if (strcmp(gtk_widget_get_name(row), identity) == 0) {
			duplicate = TRUE;
			break;
		}
		
		list = list->next;
	}
	
	if (!duplicate) {
		GtkWidget* contact = hdy_action_row_new();
		gtk_widget_set_name(contact, identity);
		
		hdy_preferences_row_set_title(HDY_PREFERENCES_ROW(contact), name);
		hdy_action_row_set_subtitle(HDY_ACTION_ROW(contact), identity);
		hdy_action_row_set_icon_name(HDY_ACTION_ROW(contact), "avatar-default-symbolic\0");
		
		gtk_container_add(GTK_CONTAINER(gui->id_search.list), contact);
		
		gtk_widget_show_all(contact);
	}
}

static void CGTK_id_search_destroy(GtkWidget* dialog, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	memset(&(gui->id_search), 0, sizeof(gui->id_search));
}

static void CGTK_id_search_dialog(GtkWidget* search_button, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;

#ifdef HANDY_USE_ZERO_API
	if (gui->new_contact.dialog) {
		gui->id_search.dialog = hdy_dialog_new(GTK_WINDOW(gui->new_contact.dialog));
	} else
	if (gui->new_group.dialog) {
		gui->id_search.dialog = hdy_dialog_new(GTK_WINDOW(gui->new_group.dialog));
	} else {
		return;
	}
#else
	gui->id_search.dialog = gtk_dialog_new();
	
	if (gui->new_contact.dialog) {
		gtk_window_set_transient_for(GTK_WINDOW(gui->id_search.dialog), GTK_WINDOW(gui->new_contact.dialog));
	} else
	if (gui->new_group.dialog) {
		gtk_window_set_transient_for(GTK_WINDOW(gui->id_search.dialog), GTK_WINDOW(gui->new_group.dialog));
	} else {
		gtk_window_set_transient_for(GTK_WINDOW(gui->id_search.dialog), GTK_WINDOW(gui->main.window));
	}
	
	gtk_window_set_modal(GTK_WINDOW(gui->id_search.dialog), TRUE);
#endif
	
	gtk_window_set_title(GTK_WINDOW(gui->id_search.dialog), "Search contact\0");
	gtk_widget_set_size_request(gui->id_search.dialog, 320, 0);
	
	GtkWidget* search_box = gtk_dialog_get_content_area(GTK_DIALOG(gui->id_search.dialog));
	gtk_box_set_spacing(GTK_BOX(search_box), 2);
	gtk_widget_set_margin_start(search_box, 4);
	gtk_widget_set_margin_bottom(search_box, 4);
	gtk_widget_set_margin_end(search_box, 4);
	gtk_widget_set_margin_top(search_box, 4);
	gtk_widget_set_vexpand(search_box, TRUE);
	
	gui->id_search.entry = gtk_entry_new();
	
	gtk_container_add(GTK_CONTAINER(search_box), gui->id_search.entry);
	
	gui->id_search.list = gtk_list_box_new();
	
	gtk_list_box_set_selection_mode(GTK_LIST_BOX(gui->id_search.list), GTK_SELECTION_BROWSE);
	gtk_widget_set_size_request(gui->id_search.list, 320, 0);
	gtk_widget_set_hexpand(gui->id_search.list, FALSE);
	gtk_widget_set_vexpand(gui->id_search.list, TRUE);
	
	GtkWidget* viewport = gtk_viewport_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(viewport), gui->id_search.list);
	
	GtkWidget* scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(
			GTK_SCROLLED_WINDOW(scrolled),
			GTK_POLICY_NEVER,
			GTK_POLICY_AUTOMATIC
	);
	
	gtk_container_add(GTK_CONTAINER(scrolled), viewport);
	gtk_container_add(GTK_CONTAINER(search_box), scrolled);
	
	g_signal_connect(gui->id_search.dialog, "destroy\0", G_CALLBACK(CGTK_id_search_destroy), gui);
	
	g_signal_connect(gui->id_search.entry, "changed\0", G_CALLBACK(CGTK_id_search_changed), gui);
	g_signal_connect(gui->id_search.list, "row-activated\0", G_CALLBACK(CGTK_id_search_select), gui);
	
	gtk_widget_show_all(gui->id_search.dialog);
}
