//
// Created by thejackimonster on 12.04.20.
//

#ifndef CADET_GTK_CHAT_H
#define CADET_GTK_CHAT_H

#include "handy_ui.h"

void CGTK_init_chat(GtkWidget* header, GtkWidget* content, GtkWidget* back_button, handy_callbacks_t callbacks);

GtkWidget* CGTK_get_chat_list(GtkWidget* content, const char* contact_id, const char* contact_port);

void CGTK_load_chat(GtkWidget* header, GtkWidget* content, GtkListBoxRow* row);

void CGTK_add_message(GtkWidget* chat_list, const char* msg_text, gboolean my_message, const char* sender);

#endif //CADET_GTK_CHAT_H
