//
// Created by thejackimonster on 17.06.20.
//

#include "upload.h"

const char* CGTK_generate_upload_path(const char* extension) {
	const char* filename = CGTK_generate_random_filename();
	const char* path = CGTK_storage_file_path(CGTK_STORAGE_UPLOAD_DIR, filename);
	
	static char upload_path [CGTK_PATH_SIZE];
	
	strncpy(upload_path, path, CGTK_PATH_SIZE);
	
	if (extension) {
		const size_t offset = strlen(path);
		
		strncpy(upload_path + offset, extension, CGTK_PATH_SIZE - offset);
	}
	
	return upload_path;
}

const char* CGTK_upload_file_from(const char* path) {
	if (!CGTK_check_existence(path)) {
		return NULL;
	}
	
	const char* extension = CGTK_get_extension(path);
	const char* upload_path = CGTK_generate_upload_path(extension);
	
	if (CGTK_check_directory(path)) {
		return NULL; // directories can be compressed later (for now unsupported)
	} else
	if (CGTK_copy_file(path, upload_path) != 0) {
		return NULL;
	}
	
	return upload_path;
}
