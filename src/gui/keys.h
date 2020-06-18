//
// Created by thejackimonster on 17.06.20.
//

#ifndef CADET_GTK_GUI_KEYS_H
#define CADET_GTK_GUI_KEYS_H

#include "../gui.h"

void CGTK_keys_add(cgtk_chat_t* chat, const char* data);

int CGTK_keys_pick(cgtk_chat_t* chat, const char* path, const cgtk_file_t* file);

void CGTK_keys_clear(cgtk_chat_t* chat);

#endif //CADET_GTK_GUI_KEYS_H
