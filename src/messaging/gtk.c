//
// Created by thejackimonster on 20.05.20.
//

void CGTK_prepare_gtk(messaging_t* messaging) {
	close_if_open(&(messaging->pipe_gnunet[0]));
	close_if_open(&(messaging->pipe_gtk[1]));
}

void CGTK_send_gtk_identity(messaging_t* messaging, const struct GNUNET_PeerIdentity* identity) {
	msg_type_t type = MSG_GTK_IDENTITY;
	
	write(messaging->pipe_gtk[1], &type, sizeof(type));
	write(messaging->pipe_gtk[1], identity, sizeof(struct GNUNET_PeerIdentity));
}

void CGTK_send_gtk_found(messaging_t* messaging, const char* name, const struct GNUNET_PeerIdentity* identity) {
	GString* string = g_string_new(name);
	guint hash = g_string_hash(string);
	g_string_free(string, TRUE);
	
	msg_type_t type = MSG_GTK_FOUND;
	
	write(messaging->pipe_gtk[1], &type, sizeof(type));
	write(messaging->pipe_gtk[1], &hash, sizeof(guint));
	write(messaging->pipe_gtk[1], identity, sizeof(struct GNUNET_PeerIdentity));
}

void CGTK_send_gtk_connect(messaging_t* messaging, const struct GNUNET_PeerIdentity* source,
						   const struct GNUNET_HashCode* port) {
	msg_type_t type = MSG_GTK_CONNECT;
	
	write(messaging->pipe_gtk[1], &type, sizeof(type));
	write(messaging->pipe_gtk[1], source, sizeof(struct GNUNET_PeerIdentity));
	write(messaging->pipe_gtk[1], port, sizeof(struct GNUNET_HashCode));
}

void CGTK_send_gtk_disconnect(messaging_t* messaging, const struct GNUNET_PeerIdentity* source,
							  const struct GNUNET_HashCode* port) {
	msg_type_t type = MSG_GTK_DISCONNECT;
	
	write(messaging->pipe_gtk[1], &type, sizeof(type));
	write(messaging->pipe_gtk[1], source, sizeof(struct GNUNET_PeerIdentity));
	write(messaging->pipe_gtk[1], port, sizeof(struct GNUNET_HashCode));
}

ssize_t CGTK_send_gtk_message(messaging_t* messaging, const struct GNUNET_PeerIdentity* source,
							  const struct GNUNET_HashCode* port, const char* buffer, size_t length) {
	msg_type_t type = MSG_GTK_RECV_MESSAGE;
	
	write(messaging->pipe_gtk[1], &type, sizeof(type));
	write(messaging->pipe_gtk[1], source, sizeof(struct GNUNET_PeerIdentity));
	write(messaging->pipe_gtk[1], port, sizeof(struct GNUNET_HashCode));
	write(messaging->pipe_gtk[1], &length, sizeof(length));
	
	ssize_t offset = 0;
	
	while (offset < length) {
		ssize_t done = write(messaging->pipe_gtk[1], (buffer + offset), length - offset);
		
		if (done <= 0) {
			return -1;
		}
		
		offset += done;
	}
	
	return offset;
}

msg_type_t CGTK_recv_gtk_msg_type(messaging_t* messaging) {
	msg_type_t type = MSG_NONE;
	
	if (read(messaging->pipe_gnunet[0], &type, sizeof(type)) < sizeof(type)) {
		return MSG_ERROR;
	}
	
	return type;
}

const struct GNUNET_HashCode* CGTK_recv_gtk_hashcode(messaging_t* messaging) {
	static struct GNUNET_HashCode hashcode;
	
	if (read(messaging->pipe_gnunet[0], &hashcode, sizeof(hashcode)) < sizeof(hashcode)) {
		return NULL;
	}
	
	return &hashcode;
}

const struct GNUNET_PeerIdentity* CGTK_recv_gtk_identity(messaging_t* messaging) {
	static struct GNUNET_PeerIdentity identity;
	
	if (read(messaging->pipe_gnunet[0], &identity, sizeof(identity)) < sizeof(identity)) {
		return NULL;
	}
	
	return &identity;
}

size_t CGTK_recv_gtk_msg_length(messaging_t* messaging) {
	size_t length = 0;
	
	if (read(messaging->pipe_gnunet[0], &length, sizeof(length)) < sizeof(length)) {
		return 0;
	}
	
	return length;
}

ssize_t CGTK_recv_gtk_message(messaging_t* messaging, char* buffer, size_t length) {
	return read(messaging->pipe_gnunet[0], buffer, length);
}
