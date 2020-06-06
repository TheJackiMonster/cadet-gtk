//
// Created by thejackimonster on 03.06.20.
//

#ifndef CADET_GTK_STORAGE_FILES_H
#define CADET_GTK_STORAGE_FILES_H

#include "../storage.h"

#include <linux/limits.h>

const char* CGTK_storage_file_path(const char* subdir, const char* filename);

#define CGTK_FILENAME_SIZE (NAME_MAX - CGTK_FILE_EXTENSION_MAX_ESTIMATE)
#define CGTK_RANDOM_FILE_BUFFER_SIZE (CGTK_FILENAME_SIZE / 2)

const char* CGTK_generate_random_filename();

char* CGTK_burn_suffix_to_filename(char* filename, const char* suffix);

int CGTK_check_existence(const char* filename);

int CGTK_check_directory(const char* filename);

#endif //CADET_GTK_STORAGE_FILES_H
