//
// Created by thejackimonster on 03.06.20.
//

#include "files.h"

#include <sys/random.h>
#include <linux/limits.h>

#define CGTK_RANDOM_FILE_BUFFER_SIZE ((NAME_MAX - CGTK_FILE_EXTENSION_MAX_ESTIMATE) / 2)

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
	
	static char filename [CGTK_RANDOM_FILE_BUFFER_SIZE * 2 + 1];
	
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
