//
// Created by thejackimonster on 12.04.20.
//

#ifndef CADET_GTK_GUI_H
#define CADET_GTK_GUI_H

#include <gtk/gtk.h>

#include "config.h"
#include "json.h"

typedef struct {
	bool_t (*send_message)(const char* destination, const char* port, msg_t* msg);
	void (*update_host)(void);
	void (*open_group)(const char* port);
	void (*exit_chat)(const char* destination, const char* port);
} cgtk_callbacks_t;

typedef struct {
	cgtk_callbacks_t callbacks;
	
	char identity [CGTK_IDENTITY_BUFFER_SIZE];
	char port [CGTK_PORT_BUFFER_SIZE];
	
	GtkWidget* app_window;
	GtkWidget* content_leaflet;
	
	GtkWidget* contacts_list;
	
	GtkWidget* add_button;
	GtkWidget* identity_button;
	GtkWidget* back_button;
	GtkWidget* options_button;
	
	GtkWidget* chat_header;
	GtkWidget* chat_stack;
	
	GtkWidget* msg_button;
	GtkWidget* msg_text_view;
} cgtk_gui_t;

typedef enum {
	CONTACT_INACTIVE = 0,
	CONTACT_ACTIVE = 1,
	CONTACT_ACTIVE_GROUP = 2
} contact_state_t;

void CGTK_init_ui(cgtk_gui_t* gui);

void CGTK_update_identity_ui(cgtk_gui_t* gui, const char* identity);

void CGTK_update_contacts_ui(cgtk_gui_t* gui, const char* identity, const char* port, contact_state_t state);

void CGTK_update_chat_ui(cgtk_gui_t* gui, const char* identity, const char* port, const msg_t* msg);

#endif //CADET_GTK_GUI_H
