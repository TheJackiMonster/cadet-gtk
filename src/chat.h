//
// Created by thejackimonster on 12.04.20.
//

#ifndef CADET_GTK_CHAT_H
#define CADET_GTK_CHAT_H

#include <gtk/gtk.h>

void CGTK_init_chat(GtkWidget* header, GtkWidget* content, GtkWidget* back_button);

void CGTK_load_chat(GtkWidget* header, GtkWidget* content, GtkListBoxRow* row);

void CGTK_add_message(GtkWidget* chat_list, const char* msg_text, gboolean my_message, const char* sender);

#endif //CADET_GTK_CHAT_H
