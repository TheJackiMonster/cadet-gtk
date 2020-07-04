//
// Created by thejackimonster on 03.06.20.
//

#ifndef CADET_GTK_STORAGE_FILES_H
#define CADET_GTK_STORAGE_FILES_H

#include "../storage.h"

void CGTK_init_storage_directories();

const char* CGTK_home_file_path(const char* subdir, const char* filename);

const char* CGTK_storage_file_path(const char* subdir, const char* filename);

const char* CGTK_generate_random_filename();

int CGTK_check_existence(const char* path);

int CGTK_check_directory(const char* path);

int CGTK_check_storage_subdir(const char* path, const char* subdir);

const char* CGTK_get_filename(const char* path);

const char* CGTK_get_extension(const char* path);

const char* CGTK_get_filehash(const char* path);

int CGTK_copy_file(const char* src_path, const char* dst_path);

#endif //CADET_GTK_STORAGE_FILES_H
