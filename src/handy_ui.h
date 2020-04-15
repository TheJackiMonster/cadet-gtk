//
// Created by thejackimonster on 12.04.20.
//

#ifndef CADET_GTK_HANDY_UI_H
#define CADET_GTK_HANDY_UI_H

#include <gtk/gtk.h>

typedef struct {
	void (*send_message)(GtkWidget*, gpointer);
	void (*set_port)(GtkWidget*, gpointer);
} handy_callbacks_t;

void CGTK_init_ui(GtkWidget* window, handy_callbacks_t callbacks);

void CGTK_update_identity_ui(GtkWidget* window, const char* identity);

void CGTK_update_contacts_ui(GtkWidget* window, const char* identity, const char* port, gboolean active);

void CGTK_update_messages_ui(GtkWidget* window, const char* identity, const char* port, const char* message);

#endif //CADET_GTK_HANDY_UI_H
