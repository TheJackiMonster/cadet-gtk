//
// Created by thejackimonster on 06.06.20.
//

#include "../../storage.h"
#include "../files.h"

static void CGTK_file_cancel(GtkWidget* cancel_button, gpointer user_data) {
	cgtk_gui_t *gui = (cgtk_gui_t *) user_data;
	
	gtk_widget_destroy(gui->file.dialog);
}

static void CGTK_file_send(GtkWidget* send_button, gpointer user_data) {
	cgtk_gui_t *gui = (cgtk_gui_t *) user_data;
	
	GString* name = g_string_new(gtk_stack_get_visible_child_name(
			GTK_STACK(gui->chat.stack)
	));
	
	const char* destination = name->str;
	const char* port = "\0";
	
	uint index = CGTK_split_name(name, &destination, &port);
	
	GList* children = gtk_container_get_children(GTK_CONTAINER(gui->file.stack));
	
	while (children) {
		const char* path = gtk_widget_get_name(GTK_WIDGET(children->data));
		const char* filename = "\0";
		const char* upload = NULL;
		
		if (CGTK_check_existence(path)) {
			filename = CGTK_get_filename(path);
			upload = CGTK_upload_file_from(path);
		} else {
			GdkPixbuf* image = CGTK_load_image_from_file(gui, path);
			
			upload = CGTK_generate_upload_path(".jpg\0");
			
			if ((image) && (gdk_pixbuf_save(image, upload, "jpeg\0", NULL, "quality\0", "100\0", NULL))) {
				filename = "image.jpg\0";
			} else {
				upload = NULL;
			}
		}
		
		if (upload) {
			cgtk_file_description_t* desc = CGTK_get_description(gui, upload);
			
			const char* hashcode = CGTK_get_filehash(upload);
			
			if (!hashcode) {
				hashcode = "\0";
			}
			
			desc->name = g_strdup(filename);
			desc->hash = g_strdup(hashcode);
			desc->progress = 0.0f;
			
			printf("-> desc: '%s' %s %s\n", upload, desc->name, desc->hash);
			
			cgtk_1tu_key_t key;
			CGTK_generate_new_key(&key);
			
			if ((CGTK_encrypt_in_storage(upload, &key) == 0) && (CGTK_store_key_for(upload, &key) == 0)) {
				msg_t msg = {};
				msg.kind = MSG_KIND_KEY;
				msg.key.type = MSG_KEY_1TU;
				msg.key.data = CGTK_key_to_string(&key);
				
				CGTK_wipe_key(&key);
				
				if (gui->callbacks.send_message(destination, port, &msg)) {
					gui->callbacks.upload_file(upload);
				}
			} else {
				CGTK_wipe_key(&key);
			}
		}
		
		children = children->next;
	}
	
	if (name->str[index] == '\0') {
		name->str[index] = '_';
	}
	
	g_string_free(name, TRUE);
	
	gtk_widget_destroy(gui->file.dialog);
}

static void CGTK_file_remove(GtkWidget* remove_button, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	GtkWidget* current = gtk_stack_get_visible_child(GTK_STACK(gui->file.stack));
	
	if (current) {
		CGTK_unload_data_from_file(gui, gtk_widget_get_name(current));
		
		gtk_container_remove(GTK_CONTAINER(gui->file.stack), current);
	}
	
	if (gtk_container_get_children(GTK_CONTAINER(gui->file.stack)) == NULL) {
		gtk_widget_destroy(gui->file.dialog);
	}
}

static void CGTK_file_add_dynamic(cgtk_gui_t* gui, const char* filename);

static void CGTK_file_add_dialog(GtkWidget* add_button, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	GtkWidget* dialog = gtk_file_chooser_dialog_new(
			"Select files\0",
			GTK_WINDOW(gui->file.dialog),
			GTK_FILE_CHOOSER_ACTION_OPEN,
			"Cancel\0",
			GTK_RESPONSE_CANCEL,
			"Select\0",
			GTK_RESPONSE_ACCEPT,
			NULL
	);
	
	gint res = gtk_dialog_run(GTK_DIALOG(dialog));
	
	if (res == GTK_RESPONSE_ACCEPT) {
		char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		
		CGTK_file_add_dynamic(gui, filename);
		
		g_free (filename);
	}
	
	gtk_widget_destroy(dialog);
}

static void CGTK_file_animation_stop(cgtk_gui_t* gui);

static void CGTK_file_destroy(GtkWidget* dialog, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	CGTK_file_animation_stop(gui);
	
	GList* children = gtk_container_get_children(GTK_CONTAINER(gui->file.stack));
	
	while (children) {
		GtkWidget* box = GTK_WIDGET(children->data);
		
		CGTK_unload_data_from_file(gui, gtk_widget_get_name(box));
		
		children = children->next;
	}
	
	memset(&(gui->file), 0, sizeof(gui->file));
}

static void CGTK_file_dialog(cgtk_gui_t* gui) {
#ifdef HANDY_USE_ZERO_API
	gui->file.dialog = hdy_dialog_new(GTK_WINDOW(gui->main.window));
#else
	gui->file.dialog = gtk_dialog_new();
#endif
	
	gtk_window_set_title(GTK_WINDOW(gui->file.dialog), "File\0");
	gtk_widget_set_size_request(gui->file.dialog, 300, 0);
	
	GtkWidget* main_box = gtk_dialog_get_content_area(GTK_DIALOG(gui->file.dialog));
	gtk_box_set_spacing(GTK_BOX(main_box), 2);
	gtk_widget_set_margin_start(main_box, 4);
	gtk_widget_set_margin_bottom(main_box, 4);
	gtk_widget_set_margin_end(main_box, 4);
	gtk_widget_set_margin_top(main_box, 4);
	gtk_widget_set_vexpand(main_box, TRUE);
	
	GtkWidget* stack_switch_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
	
	GtkWidget* button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(button_box), GTK_BUTTONBOX_END);
	gtk_box_set_spacing(GTK_BOX(button_box), 2);
	
	gui->file.stack = gtk_stack_new();
	gtk_stack_set_transition_type(GTK_STACK(gui->file.stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
	gtk_stack_set_transition_duration(GTK_STACK(gui->file.stack), CGTK_ANIMATION_DURATION);
	
	gui->file.remove_button = gtk_button_new_from_icon_name("list-remove-symbolic\0", GTK_ICON_SIZE_MENU);
	gtk_widget_set_valign(gui->file.remove_button, GTK_ALIGN_START);
	gtk_widget_set_margin_start(gui->file.remove_button, 2);
	gtk_widget_set_margin_end(gui->file.remove_button, 2);
	
	gui->file.add_button = gtk_button_new_from_icon_name("list-add-symbolic\0", GTK_ICON_SIZE_MENU);
	gtk_widget_set_valign(gui->file.add_button, GTK_ALIGN_END);
	gtk_widget_set_margin_start(gui->file.add_button, 2);
	gtk_widget_set_margin_end(gui->file.add_button, 2);
	
	gui->file.stack_switcher = gtk_stack_switcher_new();
	gtk_stack_switcher_set_stack(GTK_STACK_SWITCHER(gui->file.stack_switcher), GTK_STACK(gui->file.stack));
	gtk_widget_set_halign(gui->file.stack_switcher, GTK_ALIGN_CENTER);
	gtk_widget_set_hexpand(gui->file.stack_switcher, TRUE);
	
	GtkWidget* cancel_button = gtk_button_new_with_label("Cancel\0");
	GtkWidget* send_button = gtk_button_new_with_label("Send\0");
	
	gtk_container_add(GTK_CONTAINER(stack_switch_box), gui->file.remove_button);
	gtk_container_add(GTK_CONTAINER(stack_switch_box), gui->file.stack_switcher);
	gtk_container_add(GTK_CONTAINER(stack_switch_box), gui->file.add_button);
	
	gtk_container_add(GTK_CONTAINER(main_box), gui->file.stack);
	gtk_container_add(GTK_CONTAINER(main_box), stack_switch_box);
	gtk_container_add(GTK_CONTAINER(main_box), button_box);
	
	gtk_box_set_child_packing(GTK_BOX(main_box), gui->file.stack, TRUE, TRUE, 2, GTK_PACK_START);
	gtk_box_set_child_packing(GTK_BOX(main_box), button_box, FALSE, FALSE, 2, GTK_PACK_END);
	
	gtk_container_add(GTK_CONTAINER(button_box), cancel_button);
	gtk_container_add(GTK_CONTAINER(button_box), send_button);
	
	gtk_box_set_child_packing(GTK_BOX(button_box), cancel_button, FALSE, FALSE, 2, GTK_PACK_START);
	gtk_box_set_child_packing(GTK_BOX(button_box), send_button, FALSE, FALSE, 2, GTK_PACK_START);
	
	g_signal_connect(gui->file.dialog, "destroy\0", G_CALLBACK(CGTK_file_destroy), gui);
	
	g_signal_connect(gui->file.remove_button, "clicked\0", G_CALLBACK(CGTK_file_remove), gui);
	g_signal_connect(gui->file.add_button, "clicked\0", G_CALLBACK(CGTK_file_add_dialog), gui);
	
	g_signal_connect(cancel_button, "clicked\0", G_CALLBACK(CGTK_file_cancel), gui);
	g_signal_connect(send_button, "clicked\0", G_CALLBACK(CGTK_file_send), gui);
	
	gtk_widget_show_all(gui->file.dialog);
}

static void CGTK_file_entry_add(cgtk_gui_t* gui, const char* filename, const char* icon_name, GtkWidget* file_viewer, gboolean show_filename) {
	GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
	gtk_widget_set_name(box, filename);
	
	GtkWidget* label = gtk_label_new(show_filename? filename : "\0");
	gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_START);
	gtk_widget_set_margin_start(label, 2);
	gtk_widget_set_margin_end(label, 2);
	gtk_widget_set_hexpand(label, TRUE);
	
	GtkWidget* entry = gtk_entry_new();
	gtk_widget_set_margin_start(entry, 2);
	gtk_widget_set_margin_end(entry, 2);
	gtk_widget_set_hexpand(entry, TRUE);
	
	gtk_container_add(GTK_CONTAINER(box), label);
	gtk_container_add(GTK_CONTAINER(box), file_viewer);
	gtk_container_add(GTK_CONTAINER(box), entry);
	
	gtk_stack_add_named(GTK_STACK(gui->file.stack), box, filename);
	
	GValue iconval = G_VALUE_INIT;
	g_value_init(&iconval, G_TYPE_STRING);
	g_value_set_string(&iconval, icon_name);
	
	gtk_container_child_set_property(GTK_CONTAINER(gui->file.stack), GTK_WIDGET(box), "icon-name\0", &iconval);
	
	g_value_unset(&iconval);
	
	gtk_widget_show_all(box);
	gtk_widget_show_all(gui->file.stack);
	gtk_widget_show_all(gui->file.stack_switcher);
	
	gtk_stack_set_visible_child_name(GTK_STACK(gui->file.stack), filename);
}

static void CGTK_file_add(cgtk_gui_t* gui, const char* filename) {
	const gboolean is_directory = CGTK_check_directory(filename);
	const char* icon_name = is_directory? "folder-symbolic\0" : "text-x-generic-symbolic\0";
	
	GtkWidget* image = gtk_image_new_from_icon_name(icon_name, GTK_ICON_SIZE_DIALOG);
	gtk_widget_set_name(image, filename);
	gtk_widget_set_hexpand(image, TRUE);
	gtk_widget_set_vexpand(image, TRUE);
	
	CGTK_file_entry_add(gui, filename, icon_name, image, TRUE);
}

static void CGTK_file_draw_fitting(GtkWidget* drawing_area, cairo_t* cairo, const GdkPixbuf* image) {
	GtkStyleContext* context = gtk_widget_get_style_context(drawing_area);
	
	const guint width = gtk_widget_get_allocated_width(drawing_area);
	const guint height = gtk_widget_get_allocated_height(drawing_area);
	
	gtk_render_background(context, cairo, 0, 0, width, height);
	
	int dwidth = gdk_pixbuf_get_width(image);
	int dheight = gdk_pixbuf_get_height(image);
	
	double ratio_width = 1.0 * width / dwidth;
	double ratio_height = 1.0 * height / dheight;
	
	const double ratio = ratio_width < ratio_height? ratio_width : ratio_height;
	
	dwidth = (int) (dwidth * ratio);
	dheight = (int) (dheight * ratio);
	
	double dx = (width - dwidth) * 0.5;
	double dy = (height - dheight) * 0.5;
	
	const int interp_type = (ratio >= 1.0? GDK_INTERP_NEAREST : GDK_INTERP_BILINEAR);
	
	GdkPixbuf* scaled = gdk_pixbuf_scale_simple(image, dwidth, dheight, interp_type);
	
	gtk_render_icon(context, cairo, scaled, dx, dy);
	
	cairo_fill(cairo);
	
	g_object_unref(scaled);
}

static void CGTK_file_animation_stop(cgtk_gui_t* gui) {
	if (gui->file.animation.redraw) {
		g_source_remove(gui->file.animation.redraw);
		gui->file.animation.redraw = 0;
	}
	
	gui->file.animation.drawing_area = NULL;
	
	if (gui->file.animation.iter) {
		g_object_unref(gui->file.animation.iter);
		
		gui->file.animation.iter = NULL;
	}
}

static int CGTK_file_redraw_animation(gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t *) user_data;
	
	gui->file.animation.redraw = 0;
	
	if (gui->file.animation.drawing_area) {
		gtk_widget_queue_draw(gui->file.animation.drawing_area);
	}
	
	return FALSE;
}

static gboolean CGTK_file_draw_animation(GtkWidget* drawing_area, cairo_t* cairo, gpointer data) {
	cgtk_gui_t* gui = (cgtk_gui_t *) data;
	
	GdkPixbufAnimation* animation = CGTK_load_animation_from_file(gui, gtk_widget_get_name(drawing_area));
	
	if ((drawing_area != gui->file.animation.drawing_area) || (!gui->file.animation.iter)) {
		CGTK_file_animation_stop(gui);
		
		gui->file.animation.iter = gdk_pixbuf_animation_get_iter(animation, NULL);
	} else {
		gdk_pixbuf_animation_iter_advance(gui->file.animation.iter, NULL);
	}
	
	gui->file.animation.drawing_area = drawing_area;
	
	GdkPixbuf* image = gdk_pixbuf_animation_iter_get_pixbuf(gui->file.animation.iter);
	
	if (image) {
		CGTK_file_draw_fitting(drawing_area, cairo, image);
	}
	
	int delay = gdk_pixbuf_animation_iter_get_delay_time(gui->file.animation.iter);
	
	gui->file.animation.redraw = g_timeout_add(delay, CGTK_file_redraw_animation, gui);
	
	return FALSE;
}

static void CGTK_file_add_animation(cgtk_gui_t* gui, const char* filename, GdkPixbufAnimation* animation) {
	CGTK_store_animation_to_file(gui, filename, animation);
	
	GtkWidget* drawing_area = gtk_drawing_area_new();
	gtk_widget_set_name(drawing_area, filename);
	gtk_widget_set_hexpand(drawing_area, TRUE);
	gtk_widget_set_vexpand(drawing_area, TRUE);
	
	g_signal_connect(drawing_area, "draw\0", G_CALLBACK(CGTK_file_draw_animation), gui);
	
	CGTK_file_entry_add(gui, filename, "video-x-generic-symbolic\0", drawing_area, TRUE);
}

static gboolean CGTK_file_draw_image(GtkWidget* drawing_area, cairo_t* cairo, gpointer data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) data;
	
	const GdkPixbuf* image = CGTK_load_image_from_file(gui, gtk_widget_get_name(drawing_area));
	
	CGTK_file_draw_fitting(drawing_area, cairo, image);
	
	return FALSE;
}

static void CGTK_file_add_image(cgtk_gui_t* gui, const char* filename, GdkPixbuf* image, gboolean show_filename) {
	CGTK_store_image_to_file(gui, filename, image);
	
	GtkWidget* drawing_area = gtk_drawing_area_new();
	gtk_widget_set_name(drawing_area, filename);
	gtk_widget_set_hexpand(drawing_area, TRUE);
	gtk_widget_set_vexpand(drawing_area, TRUE);
	
	g_signal_connect(drawing_area, "draw\0", G_CALLBACK(CGTK_file_draw_image), gui);
	
	CGTK_file_entry_add(gui, filename, "image-x-generic-symbolic\0", drawing_area, show_filename);
}

static void CGTK_file_add_image_dynamic(cgtk_gui_t* gui, GdkPixbuf* image) {
	GString* filename = g_string_new(CGTK_generate_random_filename());
	g_string_append(filename, ".jpg\0");
	
	CGTK_file_add_image(gui, filename->str, image, FALSE);
	
	g_string_free(filename, TRUE);
}

static void CGTK_file_dialog_image(cgtk_gui_t* gui, GdkPixbuf* image) {
	CGTK_file_dialog(gui);
	
	CGTK_file_add_image_dynamic(gui, image);
}

static void CGTK_file_add_dynamic(cgtk_gui_t* gui, const char* filename) {
	GdkPixbufFormat* format = gdk_pixbuf_get_file_info(filename, NULL, NULL);
	
	if (format) {
		GdkPixbufAnimation* animation = gdk_pixbuf_animation_new_from_file(filename, NULL);
		
		if (animation) {
			CGTK_file_add_animation(gui, filename, animation);
		} else {
			GdkPixbuf* image = gdk_pixbuf_new_from_file(filename, NULL);
			
			CGTK_file_add_image(gui, filename, image, TRUE);
		}
	} else {
		CGTK_file_add(gui, filename);
	}
}

static void CGTK_file_add_uri_dynamic(cgtk_gui_t* gui, const gchar* uri) {
	gchar* filename = g_filename_from_uri(uri, NULL, NULL);
	
	if (filename) {
		if (CGTK_check_existence(filename)) {
			CGTK_file_add_dynamic(gui, filename);
		}
		
		g_free(filename);
	}
}

static void CGTK_file_dialog_uris(cgtk_gui_t* gui, const gchar** uris) {
	CGTK_file_dialog(gui);
	
	const gchar** iter = uris;
	
	while (*iter) {
		CGTK_file_add_uri_dynamic(gui, *iter);
		
		iter++;
	}
}
