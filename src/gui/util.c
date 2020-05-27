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

const char* CGTK_get_entry_text(GtkWidget* entry_widget) {
	GtkEntry* entry = GTK_ENTRY(entry_widget);
	
	if (gtk_entry_get_text_length(entry) > 0) {
		return gtk_entry_get_text(entry);
	} else {
		const char* placeholder = gtk_entry_get_placeholder_text(entry);
		
		if (placeholder) {
			return placeholder;
		} else {
			return "\0";
		}
	}
}

GString* CGTK_regex_append_escaped(GString* base, const char* chars) {
	GString* escaped = base? base : g_string_new("\0");
	
	if (base) {
		g_string_append_c(escaped, '|');
	}
	
	g_string_append_c(escaped, '(');
	
	if (chars) {
		while (*chars) {
			char needle [2];
			needle[0] = *chars;
			needle[1] = '\0';
			
			// TODO: This needs more investigation how GNUnets REGEX works in detail!
			/*
			 * original haystack: "^$\\.*+?()[]{}|\0"
			 */
			if (strstr("^$\\*+?(){}|\0", needle)) {
				g_string_append_c(escaped, '\\');
			}
			
			g_string_append_c(escaped, *chars);
			chars++;
		}
	}
	
	g_string_append_c(escaped, ')');
	
	return escaped;
}
