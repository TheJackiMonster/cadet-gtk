//
// Created by thejackimonster on 12.04.20.
//

#ifndef CADET_GTK_CHAT_H
#define CADET_GTK_CHAT_H

#include "../gui.h"

void CGTK_id_search_entry_found(cgtk_gui_t* gui, const char* name, const char* identity);

void CGTK_init_chat(GtkWidget* header, GtkWidget* content, cgtk_gui_t* gui);

GtkTextBuffer* CGTK_get_chat_text_buffer(cgtk_gui_t* gui);

GtkWidget* CGTK_get_chat_list(cgtk_gui_t* gui, const char* contact_id, const char* contact_port);

GtkWidget* CGTK_get_chat_label(cgtk_gui_t* gui, const char* contact_id, const char* contact_port);

void CGTK_load_chat(cgtk_gui_t* gui, GtkListBoxRow* row);

void CGTK_unload_chat(cgtk_gui_t* gui, GtkListBoxRow* row);

void CGTK_add_message(GtkWidget* chat_list, const msg_t* msg);

#endif //CADET_GTK_CHAT_H
