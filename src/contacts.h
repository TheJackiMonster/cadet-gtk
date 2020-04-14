//
// Created by thejackimonster on 12.04.20.
//

#ifndef CADET_GTK_CONTACTS_H
#define CADET_GTK_CONTACTS_H

#include <gtk/gtk.h>

void CGTK_init_contacts(GtkWidget* header, GtkWidget* content, GtkWidget* contacts_list);

void CGTK_open_contact(GtkWidget* contacts_list, const char* identity, const char* port);

void CGTK_close_contact(GtkWidget* contacts_list, const char* identity, const char* port);

#endif //CADET_GTK_CONTACTS_H
