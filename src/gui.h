//
// Created by thejackimonster on 12.04.20.
//

#ifndef CADET_GTK_GUI_H
#define CADET_GTK_GUI_H

#include <gtk/gtk.h>
#include <stdint.h>

#include "config.h"
#include "msg.h"

typedef struct {
	void (*set_name)(const char* destination, const char* port, const char* name);
	const char* (*get_name)(const char* destination, const char* port);
	uint8_t (*send_message)(const char* destination, const char* port, msg_t* msg);
	void (*update_host)(const char* announce_regex);
	void (*search_by_name)(const char* name);
	void (*open_group)(const char* port);
	void (*exit_chat)(const char* destination, const char* port);
} cgtk_callbacks_t;

typedef struct {
	cgtk_callbacks_t callbacks;
	config_t config;
	
	struct {
		char identity [CGTK_IDENTITY_BUFFER_SIZE];
		char regex [CGTK_REGEX_BUFFER_SIZE];
	} attributes;
	
	struct {
		GtkWidget* window;
		GtkWidget* leaflet;
	} main;
	
	struct {
		GtkWidget* add_button;
		GtkWidget* identity_button;
		
		GtkWidget* list;
	} contacts;
	
	struct {
		GtkWidget* back_button;
		GtkWidget* options_button;
		
		GtkWidget* header;
		GtkWidget* stack;
		
		GtkWidget* msg_button;
		GtkWidget* msg_text_view;
	} chat;
	
	struct {
		GtkWidget* dialog;
		
		GtkWidget* identity_entry;
		GtkWidget* port_entry;
		GtkWidget* name_entry;
		GtkWidget* group_check;
	} new_contact;
	
	struct {
		GtkWidget* dialog;
		
		GtkWidget* entry;
		GtkWidget* list;
	} id_search;
	
	struct {
		GtkWidget* dialog;
		
		GtkWidget* label;
		GtkWidget* port_entry;
		GtkWidget* name_entry;
		GtkWidget* mail_entry;
		GtkWidget* phone_entry;
	} identity;
	
	struct {
		GtkWidget* dialog;
		
		GtkWidget* identity_label;
		GtkWidget* port_label;
		GtkWidget* name_entry;
	} management;
} cgtk_gui_t;

typedef enum {
	CONTACT_INACTIVE = 0,
	CONTACT_ACTIVE = 1,
	CONTACT_ACTIVE_GROUP = 2
} contact_state_t;

void CGTK_init_ui(cgtk_gui_t* gui);

void CGTK_update_id_search_ui(cgtk_gui_t* gui, guint hash, const char* identity);

void CGTK_update_identity_ui(cgtk_gui_t* gui, const char* identity);

void CGTK_update_contacts_ui(cgtk_gui_t* gui, const char* identity, const char* port, contact_state_t state);

void CGTK_update_chat_ui(cgtk_gui_t* gui, const char* identity, const char* port, const msg_t* msg);

#endif //CADET_GTK_GUI_H
