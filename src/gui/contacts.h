//
// Created by thejackimonster on 12.04.20.
//

#ifndef CADET_GTK_CONTACTS_H
#define CADET_GTK_CONTACTS_H

#include "../gui.h"

typedef enum {
	CGTK_CONTACT_PERSON = 1,
	CGTK_CONTACT_GROUP = 2,
	
	CGTK_CONTACT_UNKNOWN = 0
} contact_type_t;

void CGTK_init_contacts(GtkWidget* header, GtkWidget* content, cgtk_gui_t* gui);

void CGTK_open_contact(cgtk_gui_t* gui, const char* identity, const char* port, contact_type_t type);

void CGTK_close_contact(cgtk_gui_t* gui, const char* identity, const char* port);

#endif //CADET_GTK_CONTACTS_H
