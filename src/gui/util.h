//
// Created by thejackimonster on 04.05.20.
//

#ifndef CADET_GTK_GUI_UTIL_H
#define CADET_GTK_GUI_UTIL_H

#include <gtk/gtk.h>

uint CGTK_split_name(GString* name, const char** identity, const char** port);

GString* CGTK_merge_name(const char* identity, const char* port);

const char* CGTK_get_entry_text(GtkWidget* entry_widget);

GString* CGTK_regex_append_escaped(GString* base, const char* chars);

#endif //CADET_GTK_GUI_UTIL_H
