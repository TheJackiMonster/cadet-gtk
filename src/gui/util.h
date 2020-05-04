//
// Created by thejackimonster on 04.05.20.
//

#ifndef CADET_GTK_UTIL_H
#define CADET_GTK_UTIL_H

#include <gtk/gtk.h>

uint CGTK_split_name(GString* name, const char** identity, const char** port);

GString* CGTK_merge_name(const char* identity, const char* port);

#endif //CADET_GTK_UTIL_H
