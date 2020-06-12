//
// Created by thejackimonster on 03.06.20.
//

#ifndef CADET_GTK_STORAGE_FILES_H
#define CADET_GTK_STORAGE_FILES_H

#include "../storage.h"

#include <linux/limits.h>

void CGTK_init_storage_directories();

const char* CGTK_storage_file_path(const char* subdir, const char* filename);

#define CGTK_FILENAME_SIZE (NAME_MAX - CGTK_FILE_EXTENSION_MAX_ESTIMATE)
#define CGTK_RANDOM_FILE_BUFFER_SIZE (CGTK_FILENAME_SIZE / 2)

const char* CGTK_generate_random_filename();

const char* CGTK_hash_filename(const struct GNUNET_HashCode* hashcode);

char* CGTK_force_suffix_to_filename(char* filename, const char* suffix);

int CGTK_check_existence(const char* path);

int CGTK_check_directory(const char* path);

const char* CGTK_get_filename(const char* path);

const char* CGTK_get_extension(const char* path);

const char* CGTK_upload_via_storage(const char* local_path, const char* extension);

const char* CGTK_access_via_storage(const char* storage_path);

#endif //CADET_GTK_STORAGE_FILES_H
