//
// Created by thejackimonster on 17.06.20.
//

#include "download.h"

const char* CGTK_generate_download_path(const char* extension) {
	const char* filename = CGTK_generate_random_filename();
	const char* path = CGTK_storage_file_path(CGTK_STORAGE_DOWNLOAD_DIR, filename);
	
	static char upload_path [CGTK_PATH_SIZE];
	
	strncpy(upload_path, path, CGTK_PATH_SIZE);
	
	if (extension) {
		const size_t offset = strlen(path);
		
		strncpy(upload_path + offset, extension, CGTK_PATH_SIZE - offset);
	}
	
	return upload_path;
}

inline int CGTK_download_file_to(const char* src_path, const char* dst_path) {
	return CGTK_copy_file(src_path, dst_path);
}
