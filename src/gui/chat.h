//
// Created by thejackimonster on 12.04.20.
//

#ifndef CADET_GTK_GUI_CHAT_H
#define CADET_GTK_GUI_CHAT_H

/** @addtogroup gtk_group
 *  @{
 */

#include "../gui.h"

void CGTK_id_search_entry_found(cgtk_gui_t* gui, const char* name, const char* identity);

void CGTK_init_chat(GtkWidget* header, GtkWidget* content, cgtk_gui_t* gui);

GtkTextBuffer* CGTK_get_chat_text_buffer(cgtk_gui_t* gui);

GtkWidget* CGTK_get_chat_list(cgtk_gui_t* gui, const char* contact_id, const char* contact_port);

GtkWidget* CGTK_get_chat_label(cgtk_gui_t* gui, const char* contact_id, const char* contact_port);

void CGTK_load_chat(cgtk_gui_t* gui, const char* contact_id, const char* contact_port, gboolean silent);

void CGTK_unload_chat(cgtk_gui_t* gui, const char* contact_id, const char* contact_port, gboolean silent);

void CGTK_add_talk_message(GtkWidget* chat_list, const msg_t* talk_msg);

void CGTK_update_all_members(GtkWidget* chat_list, cgtk_chat_t* chat, const msg_t* info_msg);

void CGTK_update_member(GtkWidget* chat_list, cgtk_chat_t* chat, const msg_t* msg);

void CGTK_add_file_message(GtkWidget* chat_list, const msg_t* file_msg);

/** } */

#endif //CADET_GTK_GUI_CHAT_H
