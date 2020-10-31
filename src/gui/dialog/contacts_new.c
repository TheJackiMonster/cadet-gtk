//
// Created by thejackimonster on 22.06.20.
//

static void CGTK_new_contact_and_group_destroy(GtkWidget* dialog, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	memset(&(gui->new_contact), 0, sizeof(gui->new_contact));
	memset(&(gui->new_group), 0, sizeof(gui->new_group));
}

static void CGTK_new_contact_and_group_dialog(GtkWidget* add_button, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	GtkWidget* dialog;
	
#ifdef HANDY_USE_ZERO_API
	if (gui->main.window) {
		dialog = hdy_dialog_new(GTK_WINDOW(gui->main.window));
	} else {
		return;
	}
#else
	dialog = gtk_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(gui->main.window));
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
#endif
	
	gtk_widget_set_size_request(dialog, 320, 0);
	
	gui->new_contact.dialog = dialog;
	gui->new_group.dialog = dialog;
	
	PangoAttrList* attrList = pango_attr_list_new();
	pango_attr_list_insert(attrList, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
	
	GtkWidget* contact_label = gtk_label_new("New contact\0");
	gtk_label_set_attributes(GTK_LABEL(contact_label), attrList);
	
	GtkWidget* group_label = gtk_label_new("New group\0");
	gtk_label_set_attributes(GTK_LABEL(group_label), attrList);
	
	pango_attr_list_unref(attrList);
	
	GtkWidget* contact_main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
	GtkWidget* group_main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
	
#ifdef HANDY_USE_ZERO_API
	GtkWidget* header_bar = gtk_dialog_get_header_bar(GTK_DIALOG(dialog));
#else
	GtkWidget* header_bar = hdy_header_bar_new();
	
	gtk_window_set_titlebar(GTK_WINDOW(dialog), header_bar);
#endif
	
	GtkWidget* search_button = gtk_button_new_from_icon_name("system-search-symbolic\0", GTK_ICON_SIZE_MENU);
	
#ifdef HANDY_USE_ZERO_API
	gtk_header_bar_pack_end(GTK_HEADER_BAR(header_bar), search_button);
#else
	hdy_header_bar_pack_end(HDY_HEADER_BAR(header_bar), search_button);
#endif
	
	CGTK_init_new_contact_dialog(gui, contact_main_box);
	CGTK_init_new_group_dialog(gui, group_main_box);
	
	GtkWidget* title_stack = gtk_stack_new();
	gtk_stack_add_named(GTK_STACK(title_stack), contact_label, "contact\0");
	gtk_stack_add_named(GTK_STACK(title_stack), group_label, "group\0");
	
#ifdef HANDY_USE_ZERO_API
	gtk_header_bar_set_custom_title(GTK_HEADER_BAR(header_bar), GTK_WIDGET(title_stack));
#else
	hdy_header_bar_set_custom_title(HDY_HEADER_BAR(header_bar), GTK_WIDGET(title_stack));
#endif
	
	const gchar* contact_title = gtk_label_get_text(GTK_LABEL(contact_label));
	const gchar* group_title = gtk_label_get_text(GTK_LABEL(group_label));
	
	GtkWidget* main_stack = gtk_stack_new();
	gtk_widget_set_vexpand(main_stack, TRUE);
	
	gtk_stack_add_titled(GTK_STACK(main_stack), contact_main_box, "contact\0", contact_title);
	gtk_stack_add_titled(GTK_STACK(main_stack), group_main_box, "group\0", group_title);
	
	GValue icon_string = G_VALUE_INIT;
	g_value_init(&icon_string, G_TYPE_STRING);
	
	g_value_set_string(&icon_string, "avatar-default-symbolic\0");
	gtk_container_child_set_property(GTK_CONTAINER(main_stack), contact_main_box, "icon-name\0", &icon_string);
	
	g_value_set_string(&icon_string, "system-users-symbolic\0");
	gtk_container_child_set_property(GTK_CONTAINER(main_stack), group_main_box, "icon-name\0", &icon_string);
	
	g_value_unset(&icon_string);

#ifdef HANDY_USE_ZERO_API
	HdyViewSwitcherBar* viewSwitcherBar = hdy_view_switcher_bar_new();
	hdy_view_switcher_bar_set_stack(viewSwitcherBar, GTK_STACK(main_stack));
	hdy_view_switcher_bar_set_reveal(viewSwitcherBar, TRUE);
#else
	GtkWidget* viewSwitcherBar = hdy_view_switcher_bar_new();
	hdy_view_switcher_bar_set_stack(HDY_VIEW_SWITCHER_BAR(viewSwitcherBar), GTK_STACK(main_stack));
	hdy_view_switcher_bar_set_reveal(HDY_VIEW_SWITCHER_BAR(viewSwitcherBar), TRUE);
#endif
	
	GtkWidget* main_box = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	gtk_container_add(GTK_CONTAINER(main_box), main_stack);
	
#ifdef HANDY_USE_ZERO_API
	gtk_container_add(GTK_CONTAINER(main_box), GTK_WIDGET(viewSwitcherBar));
#else
	gtk_container_add(GTK_CONTAINER(main_box), viewSwitcherBar);
#endif
	
	g_signal_connect(dialog, "destroy\0", G_CALLBACK(CGTK_new_contact_and_group_destroy), gui);
	
	g_signal_connect(search_button, "clicked\0", G_CALLBACK(CGTK_id_search_dialog), gui);
	
	g_object_bind_property(
			main_stack,
			"visible-child-name\0",
			title_stack,
			"visible-child-name\0",
			G_BINDING_SYNC_CREATE
	);
	
	g_object_bind_property(
			gui->new_contact.identity_entry,
			"text\0",
			gui->new_group.identity_entry,
			"text\0",
			G_BINDING_BIDIRECTIONAL
	);
	
	g_object_bind_property(
			gui->new_contact.name_entry,
			"text\0",
			gui->new_group.name_entry,
			"text\0",
			G_BINDING_BIDIRECTIONAL
	);
	
	g_object_bind_property(
			gui->new_contact.port_entry,
			"text\0",
			gui->new_group.port_entry,
			"text\0",
			G_BINDING_BIDIRECTIONAL
	);
	
	gtk_widget_show_all(dialog);
}
