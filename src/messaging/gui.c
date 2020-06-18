//
// Created by thejackimonster on 20.05.20.
//

void CGTK_prepare_gui(messaging_t* messaging) {
	close_if_open(&(messaging->pipe_gnunet[0]));
	close_if_open(&(messaging->pipe_gui[1]));
}

void CGTK_send_gui_identity(messaging_t* messaging, const struct GNUNET_PeerIdentity* identity) {
#ifdef CGTK_ALL_DEBUG
	printf("MESSAGING: CGTK_send_gtk_identity()\n");
#endif
	
	msg_type_t type = MSG_GUI_IDENTITY;
	
	write(messaging->pipe_gui[1], &type, sizeof(type));
	write(messaging->pipe_gui[1], identity, sizeof(struct GNUNET_PeerIdentity));
}

void CGTK_send_gui_found(messaging_t* messaging, const char* name, const struct GNUNET_PeerIdentity* identity) {
#ifdef CGTK_ALL_DEBUG
	printf("MESSAGING: CGTK_send_gtk_found()\n");
#endif
	
	GString* string = g_string_new(name);
	guint hash = g_string_hash(string);
	g_string_free(string, TRUE);
	
	msg_type_t type = MSG_GUI_FOUND;
	
	write(messaging->pipe_gui[1], &type, sizeof(type));
	write(messaging->pipe_gui[1], &hash, sizeof(guint));
	write(messaging->pipe_gui[1], identity, sizeof(struct GNUNET_PeerIdentity));
}

void CGTK_send_gui_connect(messaging_t* messaging, const struct GNUNET_PeerIdentity* source,
						   const struct GNUNET_HashCode* port) {
#ifdef CGTK_ALL_DEBUG
	printf("MESSAGING: CGTK_send_gui_connect()\n");
#endif
	
	msg_type_t type = MSG_GUI_CONNECT;
	
	write(messaging->pipe_gui[1], &type, sizeof(type));
	write(messaging->pipe_gui[1], source, sizeof(struct GNUNET_PeerIdentity));
	write(messaging->pipe_gui[1], port, sizeof(struct GNUNET_HashCode));
}

void CGTK_send_gui_disconnect(messaging_t* messaging, const struct GNUNET_PeerIdentity* source,
							  const struct GNUNET_HashCode* port) {
#ifdef CGTK_ALL_DEBUG
	printf("MESSAGING: CGTK_send_gui_disconnect()\n");
#endif
	
	msg_type_t type = MSG_GUI_DISCONNECT;
	
	write(messaging->pipe_gui[1], &type, sizeof(type));
	write(messaging->pipe_gui[1], source, sizeof(struct GNUNET_PeerIdentity));
	write(messaging->pipe_gui[1], port, sizeof(struct GNUNET_HashCode));
}

ssize_t CGTK_send_gui_message(messaging_t* messaging, const struct GNUNET_PeerIdentity* source,
							  const struct GNUNET_HashCode* port, const char* buffer, size_t length) {
#ifdef CGTK_ALL_DEBUG
	printf("MESSAGING: CGTK_send_gui_message()\n");
#endif
	
	msg_type_t type = MSG_GUI_RECV_MESSAGE;
	
	write(messaging->pipe_gui[1], &type, sizeof(type));
	write(messaging->pipe_gui[1], source, sizeof(struct GNUNET_PeerIdentity));
	write(messaging->pipe_gui[1], port, sizeof(struct GNUNET_HashCode));
	write(messaging->pipe_gui[1], &length, sizeof(length));
	
	ssize_t offset = 0;
	
	while (offset < length) {
		ssize_t done = write(messaging->pipe_gui[1], (buffer + offset), length - offset);
		
		if (done <= 0) {
			return -1;
		}
		
		offset += done;
	}
	
	return offset;
}

ssize_t CGTK_send_gui_file_progress(messaging_t* messaging, float progress, const char* path) {
#ifdef CGTK_ALL_DEBUG
	printf("MESSAGING: CGTK_send_gui_file_progress()\n");
#endif
	
	const size_t path_len = strlen(path);
	
	msg_type_t type = MSG_GUI_FILE_PROGRESS;
	
	ssize_t result = 0;
	
	result += write(messaging->pipe_gui[1], &type, sizeof(type));
	result += write(messaging->pipe_gui[1], &progress, sizeof(progress));
	result += write(messaging->pipe_gui[1], &path_len, sizeof(path_len));
	result += write(messaging->pipe_gui[1], path, path_len);
	
	return result;
}

ssize_t CGTK_send_gui_file_complete(messaging_t* messaging, bool upload, const char* path, const struct GNUNET_FS_Uri* uri) {
#ifdef CGTK_ALL_DEBUG
	printf("MESSAGING: CGTK_send_gui_file_complete()\n");
#endif
	
	const size_t path_len = strlen(path);
	
	char* suri = GNUNET_FS_uri_to_string(uri);
	const size_t uri_len = strlen(suri);
	
	msg_type_t type = MSG_GUI_FILE_COMPLETE;
	
	ssize_t result = 0;
	
	result += write(messaging->pipe_gui[1], &type, sizeof(type));
	result += write(messaging->pipe_gui[1], &upload, sizeof(upload));
	result += write(messaging->pipe_gui[1], &path_len, sizeof(path_len));
	result += write(messaging->pipe_gui[1], path, path_len);
	result += write(messaging->pipe_gui[1], &uri_len, sizeof(uri_len));
	result += write(messaging->pipe_gui[1], suri, uri_len);
	
	GNUNET_free(suri);
	
	return result;
}

msg_type_t CGTK_recv_gui_msg_type(messaging_t* messaging) {
	msg_type_t type = MSG_NONE;
	
	if (read(messaging->pipe_gnunet[0], &type, sizeof(type)) < sizeof(type)) {
		return MSG_ERROR;
	}
	
	return type;
}

uint8_t CGTK_recv_gui_code(messaging_t* messaging) {
	uint8_t code;
	read(messaging->pipe_gnunet[0], &code, sizeof(code));
	return code;
}

const struct GNUNET_HashCode* CGTK_recv_gui_hashcode(messaging_t* messaging) {
	static struct GNUNET_HashCode hashcode;
	
	if (read(messaging->pipe_gnunet[0], &hashcode, sizeof(hashcode)) < sizeof(hashcode)) {
		return NULL;
	}
	
	return &hashcode;
}

const struct GNUNET_PeerIdentity* CGTK_recv_gui_identity(messaging_t* messaging) {
	static struct GNUNET_PeerIdentity identity;
	
	if (read(messaging->pipe_gnunet[0], &identity, sizeof(identity)) < sizeof(identity)) {
		return NULL;
	}
	
	return &identity;
}

size_t CGTK_recv_gui_msg_length(messaging_t* messaging) {
	size_t length = 0;
	
	if (read(messaging->pipe_gnunet[0], &length, sizeof(length)) < sizeof(length)) {
		return 0;
	}
	
	return length;
}

ssize_t CGTK_recv_gui_message(messaging_t* messaging, char* buffer, size_t length) {
	return read(messaging->pipe_gnunet[0], buffer, length);
}

const char* CGTK_recv_gui_path(messaging_t* messaging) {
	size_t path_len = CGTK_recv_gui_msg_length(messaging);
	
	if (path_len > PATH_MAX) {
		path_len = PATH_MAX;
	}
	
	static char path [PATH_MAX + 1];
	
	if (CGTK_recv_gui_message(messaging, path, path_len) < path_len) {
		return NULL;
	}
	
	path[path_len] = '\0';
	
	return path;
}

struct GNUNET_FS_Uri* CGTK_recv_gui_uri(messaging_t* messaging) {
	const char* path = CGTK_recv_gui_path(messaging);
	
	if (path) {
		return GNUNET_FS_uri_parse(path, NULL);
	} else {
		return NULL;
	}
}
