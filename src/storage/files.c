//
// Created by thejackimonster on 03.06.20.
//

#include "files.h"

#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <sys/random.h>

const char* CGTK_storage_file_path(const char* subdir, const char* filename) {
	const struct passwd* pw = getpwuid(getuid());
	const char* home = pw->pw_dir;
	
	const size_t home_len = strlen(home);
	const size_t subdir_len = strlen(subdir);
	const size_t filename_len = strlen(filename);
	
	static const size_t storage_path_len = strlen(CGTK_STORAGE_PATH);
	static char path [PATH_MAX];
	
	size_t offset = 0, remaining = PATH_MAX;
	size_t i;
	
	for (i = 0; i < home_len; i++) {
		path[offset++] = home[i];
	}
	
	remaining -= home_len;
	
	strncpy(path + offset, CGTK_STORAGE_PATH, remaining);
	remaining = (remaining - storage_path_len > 0? remaining - storage_path_len : 0);
	offset += storage_path_len;
	
	strncpy(path + offset, subdir, remaining);
	remaining = (remaining - subdir_len > 0? remaining - subdir_len : 0);
	offset += subdir_len;
	
	strncpy(path + offset, filename, remaining);
	
	path[PATH_MAX - 1] = '\0';
	
	return path;
}

const char* CGTK_generate_random_filename() {
	char random_buffer [CGTK_RANDOM_FILE_BUFFER_SIZE];
	size_t offset = 0;
	
	while (offset < CGTK_RANDOM_FILE_BUFFER_SIZE) {
		ssize_t buffer_read = getrandom(random_buffer + offset, CGTK_RANDOM_FILE_BUFFER_SIZE - offset, 0);
		
		if (buffer_read <= 0) {
			break;
		}
		
		offset += buffer_read;
	}
	
	static char filename [CGTK_FILENAME_SIZE + 1];
	
	for (size_t i = 0; i < offset; i++) {
		const char value = random_buffer[i / 2];
		
		u_int8_t digit = ((value >> ((i & 1) << 2)) & 0xF);
		
		if (digit < 10) {
			filename[i] = ('0' + digit);
		} else {
			filename[i] = ('A' + digit - 10);
		}
	}
	
	filename[offset] = '\0';
	
	return filename;
}

char* CGTK_burn_suffix_to_filename(char* filename, const char* suffix) {
	const size_t orig_len = strlen(filename);
	const size_t suffix_len = strlen(suffix);
	
	size_t start = orig_len;
	
	if (start > CGTK_FILENAME_SIZE - suffix_len) {
		start = CGTK_FILENAME_SIZE - suffix_len;
	}
	
	for (size_t i = 0; i < suffix_len; i++) {
		filename[start + i] ^= suffix[i];
	}
	
	filename[start + suffix_len] = '\0';
	
	return filename;
}
