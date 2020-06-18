//
// Created by thejackimonster on 12.04.20.
//

#include "chat.h"

#ifdef HANDY_USE_ZERO_API
#include <libhandy-0.0/handy.h>
#else
#include <libhandy-1/handy.h>
#endif

#include "keys.h"
#include "util.h"

#include "dialog/chat_file.c"
#include "dialog/chat_management.c"

static void CGTK_back(GtkWidget* back_button, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	hdy_leaflet_set_visible_child_name(HDY_LEAFLET(gui->main.leaflet), "contacts\0");
}

static void CGTK_drag_data_received_message(GtkWidget* msg_view, GdkDragContext* context, gint x, gint y,
		GtkSelectionData* data, guint info, guint time, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	guchar* text = gtk_selection_data_get_text(data);
	
	gboolean success = FALSE;
	
	if (text) {
		gchar** uris = g_uri_list_extract_uris((gchar*) text);
		
		if (uris) {
			CGTK_file_dialog_uris(gui, (const gchar**) uris);
			
			success = TRUE;
			
			g_strfreev(uris);
		}
		
		g_free(text);
	}
	
	gtk_drag_finish(context, success, FALSE, time);
	
	gtk_text_view_set_editable(GTK_TEXT_VIEW(msg_view), FALSE);
}

static void CGTK_after_drag_data_received_message(GtkWidget* msg_view, GdkDragContext* context, gint x, gint y,
		GtkSelectionData* data, guint info, guint time, gpointer user_data) {
	gtk_text_view_set_editable(GTK_TEXT_VIEW(msg_view), TRUE);
}

static void CGTK_paste_message(GtkWidget* msg_view, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	GtkClipboard* clipboard = gtk_widget_get_clipboard(msg_view, GDK_SELECTION_CLIPBOARD);
	GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(msg_view));
	
	if (gtk_clipboard_wait_is_image_available(clipboard)) {
		GdkPixbuf* image = gtk_clipboard_wait_for_image(clipboard);
		
		CGTK_file_dialog_image(gui, image);
	} else
	if (gtk_clipboard_wait_is_uris_available(clipboard)) {
		gchar** uris = gtk_clipboard_wait_for_uris(clipboard);
		
		if (uris) {
			CGTK_file_dialog_uris(gui, (const gchar**) uris);
			
			g_strfreev(uris);
		}
	} else
	if (gtk_clipboard_wait_is_text_available(clipboard)) {
		gchar* text = gtk_clipboard_wait_for_text(clipboard);
		
		if (text) {
			const gint text_len = strlen(text);
			
			gtk_text_buffer_insert_at_cursor(buffer, text, text_len);
			
			g_free(text);
		}
	}
	
	gtk_text_view_set_editable(GTK_TEXT_VIEW(msg_view), FALSE);
}

static void CGTK_after_paste_message(GtkWidget* msg_view, gpointer user_data) {
	gtk_text_view_set_editable(GTK_TEXT_VIEW(msg_view), TRUE);
}

static void CGTK_attach_file(GtkWidget* file_button, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	CGTK_file_dialog(gui);
	CGTK_file_add_dialog(gui->file.add_button, gui);
	
	if (gtk_container_get_children(GTK_CONTAINER(gui->file.stack)) == NULL) {
		gtk_widget_destroy(gui->file.dialog);
	}
}

static void CGTK_send_message(GtkWidget* msg_button, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	GtkTextBuffer* text_buffer = CGTK_get_chat_text_buffer(gui);
	
	if (gtk_text_buffer_get_char_count(text_buffer) > 0) {
		GString* name = g_string_new(gtk_stack_get_visible_child_name(
				GTK_STACK(gui->chat.stack)
		));
		
		const char* destination = name->str;
		const char* port = "\0";
		
		uint index = CGTK_split_name(name, &destination, &port);
		
		msg_t msg = {};
		msg.kind = MSG_KIND_TALK;
		
		GtkTextIter start_iter, end_iter;
		
		gtk_text_buffer_get_start_iter(text_buffer, &start_iter);
		gtk_text_buffer_get_end_iter(text_buffer, &end_iter);
		
		msg.talk.content = gtk_text_buffer_get_text(text_buffer, &start_iter, &end_iter, FALSE);
		
		if (gui->callbacks.send_message(destination, port, &msg)) {
			gtk_text_buffer_set_text(text_buffer, "\0", 0);
		}
		
		if (name->str[index] == '\0') {
			name->str[index] = '_';
		}
		
		g_string_free(name, TRUE);
	}
}

void CGTK_init_chat(GtkWidget* header, GtkWidget* content, cgtk_gui_t* gui) {
	gtk_header_bar_set_title(GTK_HEADER_BAR(header), "Chat\0");
	gtk_header_bar_set_has_subtitle(GTK_HEADER_BAR(header), TRUE);
	gtk_header_bar_set_subtitle(GTK_HEADER_BAR(header), "\0");
	gtk_widget_set_hexpand(header, TRUE);
	
	gui->chat.back_button = gtk_button_new_from_icon_name("go-previous-symbolic\0", GTK_ICON_SIZE_MENU);
	
	GtkWidget* options = gtk_popover_menu_new();
	GtkWidget* options_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
	
	// TODO: add options for different chats
	
	GtkWidget* option_manage = gtk_button_new_with_label("Manage Chat\0");
	gtk_button_set_relief(GTK_BUTTON(option_manage), GTK_RELIEF_NONE);
	
	GtkWidget* option_files = gtk_button_new_with_label("Shared Files\0");
	gtk_button_set_relief(GTK_BUTTON(option_files), GTK_RELIEF_NONE);
	
	gtk_container_add(GTK_CONTAINER(options_box), option_manage);
	gtk_container_add(GTK_CONTAINER(options_box), option_files);
	gtk_container_add(GTK_CONTAINER(options), options_box);
	
	gtk_widget_show_all(options_box);
	
	GtkWidget* options_icon = gtk_image_new_from_icon_name("view-more-symbolic\0", GTK_ICON_SIZE_MENU);
	
	gui->chat.options_button = gtk_menu_button_new();
	gtk_menu_button_set_popover(GTK_MENU_BUTTON(gui->chat.options_button), options);
	gtk_button_set_image(GTK_BUTTON(gui->chat.options_button), options_icon);
	gtk_widget_set_sensitive(gui->chat.options_button, FALSE);
	
	gtk_header_bar_pack_start(GTK_HEADER_BAR(header), gui->chat.back_button);
	gtk_header_bar_pack_end(GTK_HEADER_BAR(header), gui->chat.options_button);
	
	gtk_widget_set_hexpand(content, TRUE);
	gtk_widget_set_vexpand(content, TRUE);
	
	gui->chat.stack = gtk_stack_new();
	gtk_stack_set_transition_type(GTK_STACK(gui->chat.stack), GTK_STACK_TRANSITION_TYPE_OVER_DOWN_UP);
	gtk_stack_set_transition_duration(GTK_STACK(gui->chat.stack), CGTK_ANIMATION_DURATION);
	
	gtk_widget_set_hexpand(gui->chat.stack, TRUE);
	gtk_widget_set_vexpand(gui->chat.stack, TRUE);
	
	GtkWidget* msg_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
	
	gui->chat.msg_text_view = gtk_text_view_new();
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(gui->chat.msg_text_view), GTK_WRAP_WORD_CHAR);
	gtk_text_view_set_input_purpose(GTK_TEXT_VIEW(gui->chat.msg_text_view), GTK_INPUT_PURPOSE_FREE_FORM);
	gtk_text_view_set_input_hints(GTK_TEXT_VIEW(gui->chat.msg_text_view), GTK_INPUT_HINT_EMOJI);
	gtk_text_view_set_left_margin(GTK_TEXT_VIEW(gui->chat.msg_text_view), 4);
	gtk_text_view_set_bottom_margin(GTK_TEXT_VIEW(gui->chat.msg_text_view), 4);
	gtk_text_view_set_right_margin(GTK_TEXT_VIEW(gui->chat.msg_text_view), 4);
	gtk_text_view_set_top_margin(GTK_TEXT_VIEW(gui->chat.msg_text_view), 4);
	gtk_widget_set_margin_bottom(gui->chat.msg_text_view, 2);
	gtk_widget_set_margin_top(gui->chat.msg_text_view, 2);
	gtk_widget_set_sensitive(gui->chat.msg_text_view, FALSE);
	
	GtkWidget* msg_scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(msg_scrolled), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_propagate_natural_height(GTK_SCROLLED_WINDOW(msg_scrolled), TRUE);
	gtk_scrolled_window_set_max_content_height(GTK_SCROLLED_WINDOW(msg_scrolled), 150);
	gtk_widget_set_hexpand(msg_scrolled, TRUE);
	gtk_widget_set_margin_start(msg_scrolled, 4);
	gtk_widget_set_margin_bottom(msg_scrolled, 2);
	gtk_widget_set_margin_end(msg_scrolled, 2);
	gtk_widget_set_margin_top(msg_scrolled, 2);
	
	GtkWidget* button_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
	
	gui->chat.file_button = gtk_button_new_from_icon_name("mail-attachment-symbolic\0", GTK_ICON_SIZE_MENU);
	gtk_widget_set_margin_start(gui->chat.file_button, 2);
	gtk_widget_set_margin_bottom(gui->chat.file_button, 2);
	gtk_widget_set_margin_end(gui->chat.file_button, 4);
	gtk_widget_set_margin_top(gui->chat.file_button, 4);
	gtk_widget_set_sensitive(gui->chat.file_button, FALSE);
	
	gui->chat.msg_button = gtk_button_new_from_icon_name("document-send\0", GTK_ICON_SIZE_MENU);
	gtk_widget_set_margin_start(gui->chat.msg_button, 2);
	gtk_widget_set_margin_bottom(gui->chat.msg_button, 4);
	gtk_widget_set_margin_end(gui->chat.msg_button, 4);
	gtk_widget_set_margin_top(gui->chat.msg_button, 2);
	gtk_widget_set_sensitive(gui->chat.msg_button, FALSE);
	
	gtk_container_add(GTK_CONTAINER(button_box), gui->chat.file_button);
	gtk_container_add(GTK_CONTAINER(button_box), gui->chat.msg_button);
	
	gtk_container_add(GTK_CONTAINER(msg_scrolled), gui->chat.msg_text_view);
	gtk_container_add(GTK_CONTAINER(msg_box), msg_scrolled);
	gtk_container_add(GTK_CONTAINER(msg_box), button_box);
	
	gtk_container_add(GTK_CONTAINER(content), gui->chat.stack);
	gtk_container_add(GTK_CONTAINER(content), msg_box);
	
	GtkSizeGroup* sizeGroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	gtk_size_group_add_widget(sizeGroup, header);
	gtk_size_group_add_widget(sizeGroup, content);
	
	g_signal_connect(gui->chat.msg_text_view, "drag-data-received\0", G_CALLBACK(CGTK_drag_data_received_message), gui);
	g_signal_connect_after(gui->chat.msg_text_view, "drag-data-received\0", G_CALLBACK(CGTK_after_drag_data_received_message), gui);
	
	g_signal_connect(gui->chat.msg_text_view, "paste-clipboard\0", G_CALLBACK(CGTK_paste_message), gui);
	g_signal_connect_after(gui->chat.msg_text_view, "paste-clipboard\0", G_CALLBACK(CGTK_after_paste_message), gui);
	
	g_signal_connect(gui->chat.file_button, "clicked\0", G_CALLBACK(CGTK_attach_file), gui);
	g_signal_connect(gui->chat.msg_button, "clicked\0", G_CALLBACK(CGTK_send_message), gui);
	g_signal_connect(option_manage, "clicked\0", G_CALLBACK(CGTK_management_dialog), gui);
	g_signal_connect(gui->chat.back_button, "clicked\0", G_CALLBACK(CGTK_back), gui);
}

GtkTextBuffer* CGTK_get_chat_text_buffer(cgtk_gui_t* gui) {
	return gtk_text_view_get_buffer(GTK_TEXT_VIEW(gui->chat.msg_text_view));
}

GtkWidget* CGTK_get_chat_list(cgtk_gui_t* gui, const char* contact_id, const char* contact_port) {
	GString* name = CGTK_merge_name(contact_id, contact_port);
	
	GtkWidget* chat_box = gtk_stack_get_child_by_name(GTK_STACK(gui->chat.stack), name->str);
	GtkWidget* chat_list;
	
	if (!chat_box) {
		chat_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
		
		GtkWidget* chat_label = gtk_label_new(contact_port);
		
		chat_list = gtk_list_box_new();
		gtk_widget_set_name(chat_list, name->str);
		gtk_list_box_set_selection_mode(GTK_LIST_BOX(chat_list), GTK_SELECTION_NONE);
		
		GtkWidget* viewport = gtk_viewport_new(NULL, NULL);
		gtk_container_add(GTK_CONTAINER(viewport), chat_list);
		
		GtkWidget* scrolled = gtk_scrolled_window_new(NULL, NULL);
		gtk_widget_set_vexpand(scrolled, TRUE);
		gtk_scrolled_window_set_policy(
				GTK_SCROLLED_WINDOW(scrolled),
				GTK_POLICY_NEVER,
				GTK_POLICY_AUTOMATIC
		);
		
		gtk_container_add(GTK_CONTAINER(scrolled), viewport);
		
		gtk_container_add(GTK_CONTAINER(chat_box), chat_label);
		gtk_container_add(GTK_CONTAINER(chat_box), scrolled);
		
		gtk_stack_add_named(GTK_STACK(gui->chat.stack), chat_box, name->str);
	} else {
		GtkWidget* scrolled = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(chat_box))->next->data);
		GtkWidget* viewport = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(scrolled))->data);
		
		chat_list = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(viewport))->data);
	}
	
	g_string_free(name, TRUE);
	
	return chat_list;
}

GtkWidget* CGTK_get_chat_label(cgtk_gui_t* gui, const char* contact_id, const char* contact_port) {
	GString* name = CGTK_merge_name(contact_id, contact_port);
	
	GtkWidget* chat_box = gtk_stack_get_child_by_name(GTK_STACK(gui->chat.stack), name->str);
	
	if (!chat_box) {
		CGTK_get_chat_list(gui, contact_id, contact_port);
	}
	
	chat_box = gtk_stack_get_child_by_name(GTK_STACK(gui->chat.stack), name->str);
	
	g_string_free(name, TRUE);
	
	return GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(chat_box))->data);
}

void CGTK_load_chat(cgtk_gui_t* gui, const char* contact_id, const char* contact_port, gboolean silent) {
	GString* name = CGTK_merge_name(contact_id, contact_port);
	
	gtk_header_bar_set_subtitle(GTK_HEADER_BAR(gui->chat.header), contact_id);
	
	GtkWidget* chat_list = CGTK_get_chat_list(gui, contact_id, contact_port);
	
	gtk_widget_show_all(chat_list);
	gtk_widget_show_all(gui->chat.stack);
	gtk_widget_show_all(gui->chat.header);
	
	gtk_stack_set_visible_child_name(GTK_STACK(gui->chat.stack), name->str);
	
	gtk_widget_set_sensitive(gui->chat.options_button, TRUE);
	gtk_widget_set_sensitive(gui->chat.msg_text_view, TRUE);
	gtk_widget_set_sensitive(gui->chat.file_button, TRUE);
	gtk_widget_set_sensitive(gui->chat.msg_button, TRUE);
	
	g_string_free(name, TRUE);
	
	if (!silent) {
		if (strcmp(hdy_leaflet_get_visible_child_name(HDY_LEAFLET(gui->main.leaflet)), "chat\0") != 0) {
			hdy_leaflet_set_visible_child_name(HDY_LEAFLET(gui->main.leaflet), "chat\0");
		}

#ifdef HANDY_USE_ZERO_API
		gboolean unfolded = (hdy_leaflet_get_fold(HDY_LEAFLET(gui->main.leaflet)) == HDY_FOLD_UNFOLDED);
#else
		gboolean unfolded = !hdy_leaflet_get_folded(HDY_LEAFLET(gui->content_leaflet));
#endif
		
		if (unfolded) {
			gtk_widget_set_visible(gui->chat.back_button, FALSE);
		}
	}
}

void CGTK_unload_chat(cgtk_gui_t* gui, const char* contact_id, const char* contact_port, gboolean silent) {
	GString* name = CGTK_merge_name(contact_id, contact_port);
	
	GtkWidget* chat_box = gtk_stack_get_child_by_name(
			GTK_STACK(gui->chat.stack), name->str
	);
	
	gtk_container_remove(GTK_CONTAINER(gui->chat.stack), chat_box);
	
	gtk_header_bar_set_subtitle(GTK_HEADER_BAR(gui->chat.header), "\0");
	
	gtk_widget_show_all(gui->chat.stack);
	gtk_widget_show_all(gui->chat.header);
	
	gtk_widget_set_sensitive(gui->chat.options_button, FALSE);
	gtk_widget_set_sensitive(gui->chat.msg_text_view, FALSE);
	gtk_widget_set_sensitive(gui->chat.file_button, FALSE);
	gtk_widget_set_sensitive(gui->chat.msg_button, FALSE);
	
	g_string_free(name, TRUE);
	
	if (!silent) {
		if (strcmp(hdy_leaflet_get_visible_child_name(HDY_LEAFLET(gui->main.leaflet)), "contacts\0") != 0) {
			hdy_leaflet_set_visible_child_name(HDY_LEAFLET(gui->main.leaflet), "contacts\0");
		}

#ifdef HANDY_USE_ZERO_API
		gboolean unfolded = (hdy_leaflet_get_fold(HDY_LEAFLET(gui->main.leaflet)) == HDY_FOLD_UNFOLDED);
#else
		gboolean unfolded = !hdy_leaflet_get_folded(HDY_LEAFLET(gui->content_leaflet));
#endif
		
		if (unfolded) {
			gtk_widget_set_visible(gui->chat.back_button, FALSE);
		}
	}
}

static void CGTK_add_message(GtkWidget* chat_list, const char* title, GtkWidget* content, uint8_t alignment) {
	const int halign = (alignment == 0? GTK_ALIGN_START : (alignment == 1? GTK_ALIGN_CENTER : GTK_ALIGN_END));
	
	GtkWidget* msg_frame = gtk_frame_new(title);
	gtk_widget_set_halign(msg_frame, halign);
	gtk_widget_set_valign(msg_frame, GTK_ALIGN_CENTER);
	gtk_frame_set_label_align(GTK_FRAME(msg_frame), alignment * 0.5f, 1.0f);
	gtk_frame_set_shadow_type(GTK_FRAME(msg_frame), GTK_SHADOW_IN);
	gtk_widget_set_size_request(msg_frame, 100, 0);
	gtk_container_set_border_width(GTK_CONTAINER(msg_frame), 4);
	
	GtkWidget* label = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(msg_frame))->data);
	gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
	
	gtk_container_add(GTK_CONTAINER(msg_frame), content);
	gtk_container_add(GTK_CONTAINER(chat_list), msg_frame);
	
	gtk_widget_show_all(chat_list);
}

static void CGTK_add_text_message(GtkWidget* chat_list, const char* title, const char* content, uint8_t alignment) {
	GtkWidget* text = gtk_label_new(content);
	gtk_label_set_line_wrap(GTK_LABEL(text), TRUE);
	gtk_label_set_line_wrap_mode(GTK_LABEL(text), PANGO_WRAP_WORD_CHAR);
	gtk_widget_set_halign(text, GTK_ALIGN_START);
	gtk_widget_set_valign(text, GTK_ALIGN_START);
	gtk_widget_set_margin_bottom(text, 4);
	gtk_widget_set_margin_start(text, 8);
	gtk_widget_set_margin_top(text, 4);
	gtk_widget_set_margin_end(text, 8);
	
	CGTK_add_message(chat_list, title, text, alignment);
}

void CGTK_add_talk_message(GtkWidget* chat_list, const msg_t* talk_msg) {
	if (talk_msg->kind == MSG_KIND_TALK) {
		switch (talk_msg->usage) {
			case MSG_USAGE_GLOBAL:
				CGTK_add_text_message(chat_list, talk_msg->talk.sender, talk_msg->talk.content, 0);
				break;
			case MSG_USAGE_LOCAL:
				CGTK_add_text_message(chat_list, talk_msg->talk.sender, talk_msg->talk.content, 2);
				break;
			default:
				break;
		}
	}
}

void CGTK_update_all_members(GtkWidget* chat_list, cgtk_chat_t* chat, const msg_t* info_msg) {
	if (chat->members) {
		g_list_free_full(chat->members, g_free);
		chat->members = NULL;
	}
	
	if (info_msg->kind == MSG_KIND_INFO) {
		const char** part = info_msg->info.participants;
		
		while (*part) {
			cgtk_member_t* member = (cgtk_member_t*) g_malloc(sizeof(cgtk_member_t));
			
			memset(member, 0, sizeof(cgtk_member_t));
			
			strncpy(member->name, *part, CGTK_NAME_BUFFER_SIZE);
			member->name[CGTK_NAME_BUFFER_SIZE - 1] = '\0';
			
			chat->members = g_list_append(chat->members, member);
			part++;
		}
	}
}

void CGTK_update_member(GtkWidget* chat_list, cgtk_chat_t* chat, const msg_t* msg) {
	if (msg->kind == MSG_KIND_JOIN) {
		if (msg->usage == MSG_USAGE_LOCAL) {
			CGTK_add_text_message(chat_list, "ENTERED\0", msg->join_leave.who, 1);
		} else {
			cgtk_member_t* member = (cgtk_member_t*) g_malloc(sizeof(cgtk_member_t));
			
			memset(member, 0, sizeof(cgtk_member_t));
			
			strncpy(member->name, msg->join_leave.who, CGTK_NAME_BUFFER_SIZE);
			member->name[CGTK_NAME_BUFFER_SIZE - 1] = '\0';
			
			chat->members = g_list_append(chat->members, member);
			
			CGTK_add_text_message(chat_list, "JOINED\0", msg->join_leave.who, 1);
		}
	} else
	if (msg->kind == MSG_KIND_LEAVE) {
		if (msg->usage == MSG_USAGE_LOCAL) {
			CGTK_add_text_message(chat_list, "QUIT\0", msg->join_leave.who, 1);
		} else {
			GList* filtered = NULL;
			GList* iter = chat->members;
			
			gboolean removed = FALSE;
			
			while (iter) {
				cgtk_member_t* member = (cgtk_member_t*) iter->data;
				
				if (strcmp(member->name, msg->join_leave.who) == 0) {
					g_free(member);
					removed = TRUE;
				} else {
					filtered = g_list_append(filtered, member);
				}
				
				iter = iter->next;
			}
			
			if (chat->members) {
				g_list_free(chat->members);
			}
			
			chat->members = filtered;
			
			if (removed) {
				CGTK_add_text_message(chat_list, "LEFT\0", msg->join_leave.who, 1);
			}
		}
	}
}

static void CGTK_open_file(GtkWidget* open_button, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	GtkWidget* file_box = gtk_widget_get_parent(open_button);
	
	const char* path = gtk_widget_get_name(file_box);
	
	GList* children = gtk_container_get_children(GTK_CONTAINER(file_box));
	
	GtkWidget* name_box = GTK_WIDGET(children->data);
	
	children = gtk_container_get_children(GTK_CONTAINER(name_box));
	
	GtkWidget* filename_label = GTK_WIDGET(children->data);
	
	const char* filename = gtk_label_get_text(GTK_LABEL(filename_label));
	
	GString* home_download_path = g_string_new(CGTK_home_file_path("/Downloads/\0", filename));
	
	if (CGTK_copy_file(path, home_download_path->str) == 0) {
		cgtk_1tu_key_t key;
		
		if (CGTK_load_key_for(path, &key) == 0) {
			CGTK_decrypt_in_storage(home_download_path->str, &key);
		}
		
		CGTK_wipe_key(&key);
		
		GString* uri = g_string_new("file://");
		g_string_append(uri, home_download_path->str);
		
		g_app_info_launch_default_for_uri(uri->str, NULL, NULL);
		
		g_string_free(uri, TRUE);
	}
	
	g_string_free(home_download_path, TRUE);
}

static void CGTK_download_file_from(GtkWidget* download_button, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	GtkWidget* file_box = gtk_widget_get_parent(download_button);
	
	const char* uri = gtk_widget_get_name(download_button);
	const char* path = gtk_widget_get_name(file_box);
	
	GtkWidget* image = gtk_button_get_image(GTK_BUTTON(download_button));
	
	gtk_image_set_from_icon_name(GTK_IMAGE(image), "media-playback-pause-symbolic\0", GTK_ICON_SIZE_LARGE_TOOLBAR);
	
	g_signal_handlers_disconnect_by_func(download_button, G_CALLBACK(CGTK_download_file_from), gui);
	
	gui->callbacks.download_file(uri, path);
	
	gtk_widget_show_all(file_box);
}

void CGTK_add_file_message(cgtk_gui_t* gui, GtkWidget* chat_list, cgtk_chat_t* chat, const msg_t* file_msg) {
	if (file_msg->kind == MSG_KIND_FILE) {
		GString* path_string = g_string_new(file_msg->file.path);
		
		gpointer match = g_hash_table_lookup(chat->files, path_string);
		
		if (match) {
			GtkWidget* file_box = GTK_WIDGET(match);
			GList* children = gtk_container_get_children(GTK_CONTAINER(file_box));
			
			GtkWidget* name_box = GTK_WIDGET(children->data);
			GtkWidget* file_button = GTK_WIDGET(children->next->data);
			
			children = gtk_container_get_children(GTK_CONTAINER(name_box));
			
			GtkWidget* progress = GTK_WIDGET(children->next->data);
			
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), file_msg->file.progress);
			
			if (file_msg->file.progress >= 1.0f) {
				cgtk_1tu_key_t key;
				gboolean unlocked = FALSE;
				
				if (CGTK_load_key_for(file_msg->file.path, &key) != 0) {
					cgtk_file_t* file = CGTK_get_file(gui, file_msg->file.path);
					
					CGTK_wipe_key(&key);
					
					if (CGTK_keys_pick(chat, file_msg->file.path, file) == 0) {
						unlocked = TRUE;
					}
				} else {
					CGTK_wipe_key(&key);
					
					unlocked = TRUE;
				}
				
				GtkWidget* image = gtk_button_get_image(GTK_BUTTON(file_button));
				
				const char* icon_name = "system-lock-screen-symbolic\0";
				
				if (unlocked) {
					icon_name = "document-open-symbolic\0";
				}
				
				gtk_image_set_from_icon_name(GTK_IMAGE(image), icon_name, GTK_ICON_SIZE_LARGE_TOOLBAR);
				
				g_signal_handlers_disconnect_by_func(file_button, G_CALLBACK(CGTK_open_file), gui);
				
				if (unlocked) {
					g_signal_connect(file_button, "clicked", G_CALLBACK(CGTK_open_file), gui);
				}
			}
			
			gtk_widget_show_all(file_box);
			
			g_string_free(path_string, TRUE);
		} else {
			GtkWidget* file_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
			gtk_widget_set_name(file_box, file_msg->file.path);
			gtk_widget_set_margin_bottom(file_box, 4);
			gtk_widget_set_margin_start(file_box, 4);
			gtk_widget_set_margin_top(file_box, 4);
			gtk_widget_set_margin_end(file_box, 4);
			
			GtkWidget* name_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
			gtk_widget_set_hexpand(name_box, TRUE);
			
			GtkWidget* filename = gtk_label_new(file_msg->file.name);
			gtk_label_set_line_wrap(GTK_LABEL(filename), TRUE);
			gtk_label_set_line_wrap_mode(GTK_LABEL(filename), PANGO_WRAP_WORD_CHAR);
			gtk_widget_set_halign(filename, GTK_ALIGN_CENTER);
			gtk_widget_set_valign(filename, GTK_ALIGN_END);
			gtk_widget_set_margin_bottom(filename, 2);
			gtk_widget_set_margin_start(filename, 8);
			gtk_widget_set_margin_top(filename, 4);
			gtk_widget_set_margin_end(filename, 8);
			
			GtkWidget* progress = gtk_progress_bar_new();
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), file_msg->file.progress);
			gtk_widget_set_halign(progress, GTK_ALIGN_CENTER);
			gtk_widget_set_valign(progress, GTK_ALIGN_START);
			gtk_widget_set_margin_bottom(progress, 4);
			gtk_widget_set_margin_start(progress, 8);
			gtk_widget_set_margin_top(progress, 2);
			gtk_widget_set_margin_end(progress, 8);
			
			const char* button_icon = "document-open-symbolic\0";
			uint8_t button_type = 0;
			
			if (file_msg->file.progress < 1.0f) {
				if (file_msg->usage == MSG_USAGE_LOCAL) {
					button_icon = "media-playback-pause-symbolic\0";
					button_type = 1;
				} else {
					button_icon = "folder-download-symbolic\0";
					button_type = 2;
				}
			}
			
			GtkWidget* file_button = gtk_button_new_from_icon_name(button_icon, GTK_ICON_SIZE_LARGE_TOOLBAR);
			gtk_button_set_relief(GTK_BUTTON(file_button), GTK_RELIEF_NONE);
			gtk_widget_set_halign(file_button, GTK_ALIGN_CENTER);
			gtk_widget_set_valign(file_button, GTK_ALIGN_END);
			gtk_widget_set_name(file_button, file_msg->file.uri);
			
			gtk_container_add(GTK_CONTAINER(name_box), filename);
			gtk_container_add(GTK_CONTAINER(name_box), progress);
			gtk_container_add(GTK_CONTAINER(file_box), name_box);
			gtk_container_add(GTK_CONTAINER(file_box), file_button);
			
			switch (button_type) {
				case 0:
					g_signal_connect(file_button, "clicked", G_CALLBACK(CGTK_open_file), gui);
					break;
				case 2:
					g_signal_connect(file_button, "clicked", G_CALLBACK(CGTK_download_file_from), gui);
					break;
				default:
					break;
			}
			
			CGTK_add_message(chat_list, file_msg->file.publisher, file_box, file_msg->usage == MSG_USAGE_LOCAL? 2 : 0);
			
			g_hash_table_insert(chat->files, path_string, file_box);
		}
	}
}
