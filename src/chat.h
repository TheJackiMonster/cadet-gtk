//
// Created by thejackimonster on 12.04.20.
//

#ifndef CADET_GTK_CHAT_H
#define CADET_GTK_CHAT_H


#include <gtk/gtk.h>

void cadet_gtk_init_chat(GtkWidget* header, GtkWidget* content, GtkWidget* back_button);

void cadet_gtk_load_chat(GtkWidget* header, GtkWidget* content, GtkListBoxRow* row);

#endif //CADET_GTK_CHAT_H
