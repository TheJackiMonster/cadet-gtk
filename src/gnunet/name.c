//
// Created by thejackimonster on 20.05.20.
//

static void CGTK_name_call(const char* name_regex) {
	if (session.name_announcement) {
		GNUNET_REGEX_announce_cancel(session.name_announcement);
	}
	
	session.name_announcement = GNUNET_REGEX_announce(
			session.cfg,
			name_regex,
			GNUNET_TIME_relative_multiply(
					GNUNET_TIME_UNIT_MINUTES,
					CGTK_GNUNET_SESSION_ANNOUNCE_DELAY_MIN
			),
			CGTK_NAME_REGEX_COMPRESSION
	);
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
	session.name_search = GNUNET_REGEX_search(
			session.cfg,
			name,
			CGTK_name_found,
			session.name_needle
	);
}
