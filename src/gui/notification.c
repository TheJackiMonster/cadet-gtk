//
// Created by thejackimonster on 02.06.20.
//

#include "notification.h"
#include "chat.h"

static void CGTK_notification_closed(NotifyNotification* notification, gpointer user_data) {
	notify_notification_clear_actions(notification);
	notify_notification_clear_hints(notification);
	
	g_object_unref(notification);
}

typedef struct cgtk_chat_notification_t {
	cgtk_gui_t* gui;
	
	char identity [CGTK_IDENTITY_BUFFER_SIZE];
	char port [CGTK_PORT_BUFFER_SIZE];
} cgtk_chat_notification_t;

static void CGTK_notification_reactivate_chat(NotifyNotification* notification, char* action, gpointer user_data) {
	cgtk_chat_notification_t* data = (cgtk_chat_notification_t*) user_data;
	
	CGTK_load_chat(data->gui, data->identity, data->port, FALSE);
	
	notify_notification_close(notification, NULL);
}

void CGTK_notification_from_chat(cgtk_gui_t* gui, const char* identity, const char* port, const msg_t* msg) {
	GString* chat_name = CGTK_merge_name(identity, port);
	
	const char* active_key = gtk_stack_get_visible_child_name(GTK_STACK(gui->chat.stack));
	const gboolean required = (active_key == NULL) || (strcmp(chat_name->str, active_key) != 0);
	
	if (required) {
		NotifyNotification* notification = NULL;
		
		switch (msg->kind) {
			case MSG_KIND_TALK: {
				notification = notify_notification_new(msg->sender, msg->content, "user-available\0");
				notify_notification_set_category(notification, "im\0");
				break;
			} case MSG_KIND_JOIN: {
				break;
			} case MSG_KIND_LEAVE: {
				break;
			} case MSG_KIND_INFO: {
				break;
			} case MSG_KIND_FILE: {
				notification = notify_notification_new(msg->publisher, msg->uri, "mail-attachment\0");
				notify_notification_set_category(notification, "transfer\0");
				break;
			} default: {
				break;
			}
		}
		
		if (notification) {
			cgtk_chat_notification_t* data = malloc(sizeof(cgtk_chat_notification_t));
			
			data->gui = gui;
			
			strncpy(data->identity, identity, CGTK_IDENTITY_BUFFER_SIZE);
			data->identity[CGTK_IDENTITY_BUFFER_SIZE - 1] = '\0';
			
			strncpy(data->port, port, CGTK_PORT_BUFFER_SIZE);
			data->port[CGTK_PORT_BUFFER_SIZE - 1] = '\0';
			
			notify_notification_add_action(
					notification, "clicked\0", "Open Chat\0",
					NOTIFY_ACTION_CALLBACK(CGTK_notification_reactivate_chat), data, free
			);
			
			g_signal_connect(notification, "closed\0", G_CALLBACK(CGTK_notification_closed), gui);
			
			notify_notification_show(notification, NULL);
		}
	}
	
	g_string_free(chat_name, TRUE);
}
