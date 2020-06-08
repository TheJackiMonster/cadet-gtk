//
// Created by thejackimonster on 12.04.20.
//

#include "chat.h"

#ifdef HANDY_USE_ZERO_API
#include <libhandy-0.0/handy.h>
#else
#include <libhandy-1/handy.h>
#endif

#include "util.h"

#include "dialog/chat_file.c"
#include "dialog/chat_management.c"

static void CGTK_back(GtkWidget* back_button, gpointer user_data) {
	cgtk_gui_t* gui = (cgtk_gui_t*) user_data;
	
	hdy_leaflet_set_visible_child_name(HDY_LEAFLET(gui->main.leaflet), "contacts\0");
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
		
		msg.content = gtk_text_buffer_get_text(text_buffer, &start_iter, &end_iter, FALSE);
		
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
	
	gtk_container_add(GTK_CONTAINER(options_box), option_manage);
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
	
	gui->chat.msg_button = gtk_button_new_from_icon_name("document-send\0", GTK_ICON_SIZE_MENU);
	gtk_widget_set_valign(gui->chat.msg_button, GTK_ALIGN_END);
	gtk_widget_set_margin_start(gui->chat.msg_button, 2);
	gtk_widget_set_margin_bottom(gui->chat.msg_button, 4);
	gtk_widget_set_margin_end(gui->chat.msg_button, 4);
	gtk_widget_set_margin_top(gui->chat.msg_button, 4);
	gtk_widget_set_sensitive(gui->chat.msg_button, FALSE);
	
	gtk_container_add(GTK_CONTAINER(msg_scrolled), gui->chat.msg_text_view);
	gtk_container_add(GTK_CONTAINER(msg_box), msg_scrolled);
	gtk_container_add(GTK_CONTAINER(msg_box), gui->chat.msg_button);
	
	gtk_container_add(GTK_CONTAINER(content), gui->chat.stack);
	gtk_container_add(GTK_CONTAINER(content), msg_box);
	
	GtkSizeGroup* sizeGroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	gtk_size_group_add_widget(sizeGroup, header);
	gtk_size_group_add_widget(sizeGroup, content);
	
	g_signal_connect(gui->chat.msg_text_view, "paste-clipboard\0", G_CALLBACK(CGTK_paste_message), gui);
	g_signal_connect_after(gui->chat.msg_text_view, "paste-clipboard\0", G_CALLBACK(CGTK_after_paste_message), gui);
	
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
	
	gtk_widget_set_sensitive(gui->chat.msg_text_view, FALSE);
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

static void CGTK_add_message(GtkWidget* chat_list, const char* title, const char* content, uint8_t alignment) {
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
	
	GtkWidget* text = gtk_label_new(content);
	gtk_label_set_line_wrap(GTK_LABEL(text), TRUE);
	gtk_label_set_line_wrap_mode(GTK_LABEL(text), PANGO_WRAP_WORD_CHAR);
	gtk_widget_set_halign(text, GTK_ALIGN_START);
	gtk_widget_set_valign(text, GTK_ALIGN_START);
	gtk_widget_set_margin_bottom(text, 4);
	gtk_widget_set_margin_start(text, 8);
	gtk_widget_set_margin_top(text, 4);
	gtk_widget_set_margin_end(text, 8);
	
	gtk_container_add(GTK_CONTAINER(msg_frame), text);
	gtk_container_add(GTK_CONTAINER(chat_list), msg_frame);
	
	gtk_widget_show_all(chat_list);
}

void CGTK_add_talk_message(GtkWidget* chat_list, const msg_t* talk_msg) {
	if (talk_msg->kind == MSG_KIND_TALK) {
		CGTK_add_message(chat_list, talk_msg->sender, talk_msg->content, talk_msg->local? 2 : 0);
	}
}

void CGTK_update_all_members(GtkWidget* chat_list, cgtk_chat_t* chat, const msg_t* info_msg) {
	if (chat->members) {
		g_list_free_full(chat->members, g_free);
		chat->members = NULL;
	}
	
	if (info_msg->kind == MSG_KIND_INFO) {
		const char** part = info_msg->participants;
		
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
		if (msg->local) {
			CGTK_add_message(chat_list, "ENTER\0", msg->who, 1);
		} else {
			cgtk_member_t* member = (cgtk_member_t*) g_malloc(sizeof(cgtk_member_t));
			
			memset(member, 0, sizeof(cgtk_member_t));
			
			strncpy(member->name, msg->who, CGTK_NAME_BUFFER_SIZE);
			member->name[CGTK_NAME_BUFFER_SIZE - 1] = '\0';
			
			chat->members = g_list_append(chat->members, member);
			
			CGTK_add_message(chat_list, "JOIN\0", msg->who, 1);
		}
	} else
	if (msg->kind == MSG_KIND_LEAVE) {
		if (msg->local) {
			CGTK_add_message(chat_list, "QUIT\0", msg->who, 1);
		} else {
			GList* filtered = NULL;
			GList* iter = chat->members;
			
			gboolean removed = FALSE;
			
			while (iter) {
				cgtk_member_t* member = (cgtk_member_t*) iter->data;
				
				if (strcmp(member->name, msg->who) == 0) {
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
				CGTK_add_message(chat_list, "LEAVE\0", msg->who, 1);
			}
		}
	}
}

void CGTK_add_file_message(GtkWidget* chat_list, const msg_t* file_msg) {
	if (file_msg->kind == MSG_KIND_FILE) {
		CGTK_add_message(chat_list, file_msg->publisher, file_msg->uri, file_msg->local? 2 : 0);
	}
}
