//
// Created by thejackimonster on 03.06.20.
//

#ifndef CADET_GTK_STORAGE_FILES_H
#define CADET_GTK_STORAGE_FILES_H

#include "../storage.h"

#include <linux/limits.h>

#define CGTK_PATH_SIZE PATH_MAX

void CGTK_init_storage_directories();

const char* CGTK_storage_file_path(const char* subdir, const char* filename);

#define CGTK_FILENAME_SIZE (NAME_MAX - CGTK_FILE_EXTENSION_MAX_ESTIMATE)
#define CGTK_RANDOM_FILE_BUFFER_SIZE (CGTK_FILENAME_SIZE / 2)

const char* CGTK_generate_random_filename();

int CGTK_check_existence(const char* path);

int CGTK_check_directory(const char* path);

const char* CGTK_get_filename(const char* path);

const char* CGTK_get_extension(const char* path);

const char* CGTK_get_filehash(const char* path);

int CGTK_copy_file(const char* src_path, const char* dst_path);

#endif //CADET_GTK_STORAGE_FILES_H
