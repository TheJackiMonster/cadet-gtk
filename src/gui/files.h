//
// Created by thejackimonster on 06.06.20.
//

#ifndef CADET_GTK_GUI_FILES_H
#define CADET_GTK_GUI_FILES_H

#include "../gui.h"

void CGTK_init_files(cgtk_files_t* files);

GdkPixbuf* CGTK_load_image_from_file(cgtk_gui_t* gui, const char* filename);

gboolean CGTK_store_image_to_file(cgtk_gui_t* gui, const char* filename, GdkPixbuf* image);

GdkPixbufAnimation* CGTK_load_animation_from_file(cgtk_gui_t* gui, const char* filename);

GdkPixbufAnimationIter* CGTK_load_animation_iter(GdkPixbufAnimation* animation);

gboolean CGTK_store_animation_to_file(cgtk_gui_t* gui, const char* filename, GdkPixbufAnimation* animation);

cgtk_file_description_t* CGTK_get_description(cgtk_gui_t* gui, const char* filename);

void CGTK_unload_data_from_file(cgtk_gui_t* gui, const char* filename);

void CGTK_free_files(cgtk_files_t* files);

#endif //CADET_GTK_GUI_FILES_H
