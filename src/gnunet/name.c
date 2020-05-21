//
// Created by thejackimonster on 20.05.20.
//

#include <gnunet/gnunet_regex_service.h>

static void CGTK_name_call(const char* name_regex) {
	if (session.name_announcement) {
		GNUNET_REGEX_announce_cancel(session.name_announcement);
	}
	
	const size_t name_regex_len = strlen(name_regex);
	
	if (name_regex_len > 0) {
		static char name_prefixed [CGTK_NAME_SEARCH_PREFIX_REG_SIZE + CGTK_NAME_SEARCH_SIZE + 5];
		
		name_prefixed[0] = '(';
		
		strncpy(name_prefixed + 1, CGTK_NAME_SEARCH_PREFIX_REG, CGTK_NAME_SEARCH_PREFIX_REG_SIZE);
		
		name_prefixed[CGTK_NAME_SEARCH_PREFIX_SIZE + 1] = ')';
		name_prefixed[CGTK_NAME_SEARCH_PREFIX_SIZE + 2] = '(';
		
		strcpy(name_prefixed + CGTK_NAME_SEARCH_PREFIX_SIZE + 3, name_regex);
		
		name_prefixed[CGTK_NAME_SEARCH_PREFIX_SIZE + name_regex_len + 3] = ')';
		name_prefixed[CGTK_NAME_SEARCH_PREFIX_SIZE + CGTK_NAME_SEARCH_SIZE + 4] = '\0';
		
		session.name_announcement = GNUNET_REGEX_announce(
				session.cfg,
				name_prefixed,
				GNUNET_TIME_relative_multiply(
						GNUNET_TIME_UNIT_MINUTES,
						CGTK_GNUNET_SESSION_ANNOUNCE_DELAY_MIN
				),
				CGTK_NAME_REGEX_COMPRESSION
		);
	} else {
		session.name_announcement = NULL;
	}
}

static void CGTK_name_found(void *cls, const struct GNUNET_PeerIdentity* identity,
		const struct GNUNET_PeerIdentity* get_path, unsigned int get_path_length,
		const struct GNUNET_PeerIdentity* put_path, unsigned int put_path_length) {
	const char* name = (const char*) cls;
	
	CGTK_send_gtk_found(messaging, name, identity);
}

static void CGTK_name_search(const char* name) {
	if (session.name_search) {
		GNUNET_REGEX_search_cancel(session.name_search);
	}
	
	if (session.name_needle) {
		GNUNET_free(session.name_needle);
	}
	
	session.name_needle = GNUNET_strdup(name);
	
	static char name_prefixed [CGTK_NAME_SEARCH_PREFIX_SIZE + CGTK_NAME_SEARCH_SIZE + 1];
	
	strcpy(name_prefixed, CGTK_NAME_SEARCH_PREFIX);
	strcpy(name_prefixed + CGTK_NAME_SEARCH_PREFIX_SIZE, name);
	
	name_prefixed[CGTK_NAME_SEARCH_PREFIX_SIZE + CGTK_NAME_SEARCH_SIZE] = '\0';
	
	session.name_search = GNUNET_REGEX_search(
			session.cfg,
			name_prefixed,
			CGTK_name_found,
			session.name_needle
	);
}
