//
// Created by thejackimonster on 04.05.20.
//

#include "util.h"

uint CGTK_split_name(GString* name, const char** identity, const char** port) {
	size_t index = 0;
	
	*identity = name->str;
	
	while (index < name->len) {
		if (name->str[index] == '_') {
			if (index + 1 < name->len) {
				*port = (name->str + index + 1);
			}
			
			name->str[index] = '\0';
			break;
		}
		
		index++;
	}
	
	return index;
}

GString* CGTK_merge_name(const char* identity, const char* port) {
	return g_string_append(g_string_append_c_inline(g_string_new(identity), '_'), port);
}
