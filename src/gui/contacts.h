//
// Created by thejackimonster on 12.04.20.
//

#ifndef CADET_GTK_GUI_CONTACTS_H
#define CADET_GTK_GUI_CONTACTS_H

#include "../gui.h"

void CGTK_init_contacts(GtkWidget* header, GtkWidget* content, cgtk_gui_t* gui);

void CGTK_open_contact(cgtk_gui_t* gui, const char* identity, const char* port);

void CGTK_reload_contact(cgtk_gui_t* gui, const char* identity, const char* port);

void CGTK_close_contact(cgtk_gui_t* gui, const char* identity, const char* port);

void CGTK_remove_contact(cgtk_gui_t* gui, const char* identity, const char* port);

#endif //CADET_GTK_GUI_CONTACTS_H
