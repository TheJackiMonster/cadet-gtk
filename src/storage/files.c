//
// Created by thejackimonster on 03.06.20.
//

#include "files.h"

#include <string.h>
#include <sys/random.h>

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
