//
// Created by thejackimonster on 17.06.20.
//

#ifndef CADET_GTK_STORAGE_DOWNLOAD_H
#define CADET_GTK_STORAGE_DOWNLOAD_H

#include "files.h"

const char* CGTK_generate_download_path(const char* extension);

int CGTK_download_file_to(const char* src_path, const char* dst_path);

#endif //CADET_GTK_STORAGE_DOWNLOAD_H
