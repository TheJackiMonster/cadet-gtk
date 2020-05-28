//
// Created by thejackimonster on 20.05.20.
//

void CGTK_prepare_gnunet(messaging_t* messaging) {
	close_if_open(&(messaging->pipe_gnunet[1]));
	close_if_open(&(messaging->pipe_gtk[0]));
}

static void CGTK_add_port_to_lookup(const char* port, size_t port_len, const struct GNUNET_HashCode* hashcode) {
	if ((port_lookup) && (!GNUNET_CONTAINER_multihashmap_contains(port_lookup, hashcode))) {
		char* port_copy = GNUNET_malloc((port_len + 1) * sizeof(char));
		strncpy(port_copy, port, port_len);
		port_copy[port_len] = '\0';
		
		GNUNET_CONTAINER_multihashmap_put(
				port_lookup,
				hashcode,
				port_copy,
				GNUNET_CONTAINER_MULTIHASHMAPOPTION_UNIQUE_ONLY
		);
	}
}

ssize_t CGTK_send_gnunet_host(messaging_t* messaging, uint8_t visibility, const char* port, const char* name_regex) {
#ifdef CGTK_ALL_DEBUG
	printf("MESSAGING: CGTK_send_gnunet_host()\n");
#endif
	
	const size_t length = (name_regex? strlen(name_regex) : 0);
	
	size_t port_len = strlen(port);
	
	struct GNUNET_HashCode hashcode;
	GNUNET_CRYPTO_hash(port, port_len, &hashcode);
	
	CGTK_add_port_to_lookup(port, port_len, &hashcode);
	
	msg_type_t type = MSG_GNUNET_HOST;
	
	write(messaging->pipe_gnunet[1], &type, sizeof(type));
	write(messaging->pipe_gnunet[1], &visibility, sizeof(visibility));
	write(messaging->pipe_gnunet[1], &hashcode, sizeof(hashcode));
	write(messaging->pipe_gnunet[1], &length, sizeof(length));
	
	ssize_t offset = 0;
	
	while (offset < length) {
		ssize_t done = write(messaging->pipe_gnunet[1], (name_regex + offset), length - offset);
		
		if (done <= 0) {
			return -1;
		}
		
		offset += done;
	}
	
	return offset;
}

ssize_t CGTK_send_gnunet_search(messaging_t* messaging, const char* name) {
#ifdef CGTK_ALL_DEBUG
	printf("MESSAGING: CGTK_send_gnunet_search()\n");
#endif
	
	const size_t length = (name? strlen(name) : 0);
	
	msg_type_t type = MSG_GNUNET_SEARCH;
	
	write(messaging->pipe_gnunet[1], &type, sizeof(type));
	write(messaging->pipe_gnunet[1], &length, sizeof(length));
	
	ssize_t offset = 0;
	
	while (offset < length) {
		ssize_t done = write(messaging->pipe_gnunet[1], (name + offset), length - offset);
		
		if (done <= 0) {
			return -1;
		}
		
		offset += done;
	}
	
	return offset;
}

ssize_t CGTK_send_gnunet_message(messaging_t* messaging, const char* destination, const char* port,
								 const char* buffer, size_t length) {
#ifdef CGTK_ALL_DEBUG
	printf("MESSAGING: CGTK_send_gnunet_message()\n");
#endif
	
	struct GNUNET_PeerIdentity identity;
	
	if (GNUNET_CRYPTO_eddsa_public_key_from_string(destination, strlen(destination), &(identity.public_key)) != GNUNET_OK) {
		return -1;
	}
	
	size_t port_len = strlen(port);
	
	struct GNUNET_HashCode hashcode;
	GNUNET_CRYPTO_hash(port, port_len, &hashcode);
	
	CGTK_add_port_to_lookup(port, port_len, &hashcode);
	
	msg_type_t type = MSG_GNUNET_SEND_MESSAGE;
	
	write(messaging->pipe_gnunet[1], &type, sizeof(type));
	write(messaging->pipe_gnunet[1], &identity, sizeof(identity));
	write(messaging->pipe_gnunet[1], &hashcode, sizeof(hashcode));
	write(messaging->pipe_gnunet[1], &length, sizeof(length));
	
	ssize_t offset = 0;
	
	while (offset < length) {
		ssize_t done = write(messaging->pipe_gnunet[1], (buffer + offset), length - offset);
		
		if (done <= 0) {
			return -1;
		}
		
		offset += done;
	}
	
	return offset;
}

void CGTK_send_gnunet_group(messaging_t* messaging, const char* port) {
#ifdef CGTK_ALL_DEBUG
	printf("MESSAGING: CGTK_send_gnunet_group()\n");
#endif
	
	size_t port_len = strlen(port);
	
	struct GNUNET_HashCode hashcode;
	GNUNET_CRYPTO_hash(port, port_len, &hashcode);
	
	CGTK_add_port_to_lookup(port, port_len, &hashcode);
	
	msg_type_t type = MSG_GNUNET_GROUP;
	
	write(messaging->pipe_gnunet[1], &type, sizeof(type));
	write(messaging->pipe_gnunet[1], &hashcode, sizeof(hashcode));
}

void CGTK_send_gnunet_exit(messaging_t* messaging, const char* destination, const char* port) {
#ifdef CGTK_ALL_DEBUG
	printf("MESSAGING: CGTK_send_gnunet_exit()\n");
#endif
	
	struct GNUNET_PeerIdentity identity;
	
	if (GNUNET_CRYPTO_eddsa_public_key_from_string(destination, strlen(destination), &(identity.public_key)) != GNUNET_OK) {
		return;
	}
	
	size_t port_len = strlen(port);
	
	struct GNUNET_HashCode hashcode;
	GNUNET_CRYPTO_hash(port, port_len, &hashcode);
	
	CGTK_add_port_to_lookup(port, port_len, &hashcode);
	
	msg_type_t type = MSG_GNUNET_EXIT;
	
	write(messaging->pipe_gnunet[1], &type, sizeof(type));
	write(messaging->pipe_gnunet[1], &identity, sizeof(identity));
	write(messaging->pipe_gnunet[1], &hashcode, sizeof(hashcode));
}

msg_type_t CGTK_recv_gnunet_msg_type(messaging_t* messaging) {
	msg_type_t type = MSG_NONE;
	
	if (read(messaging->pipe_gtk[0], &type, sizeof(type)) < sizeof(type)) {
		return MSG_ERROR;
	}
	
	return type;
}

const char* CGTK_recv_gnunet_identity(messaging_t* messaging) {
	struct GNUNET_PeerIdentity identity;
	
	if (read(messaging->pipe_gtk[0], &identity, sizeof(identity)) < sizeof(identity)) {
		return NULL;
	}
	
	return GNUNET_i2s_full(&identity);
}

const char* CGTK_recv_gnunet_port(messaging_t* messaging) {
	struct GNUNET_HashCode hashcode;
	
	if (read(messaging->pipe_gtk[0], &hashcode, sizeof(hashcode)) < sizeof(hashcode)) {
		return NULL;
	}
	
	if ((!port_lookup) || (!GNUNET_CONTAINER_multihashmap_contains(port_lookup, &hashcode))) {
		return NULL;
	}
	
	return (const char*) GNUNET_CONTAINER_multihashmap_get(port_lookup, &hashcode);
}

guint CGTK_recv_gnunet_hash(messaging_t* messaging) {
	guint hash;
	
	if (read(messaging->pipe_gtk[0], &hash, sizeof(guint)) < sizeof(guint)) {
		return 0;
	}
	
	return hash;
}

size_t CGTK_recv_gnunet_msg_length(messaging_t* messaging) {
	size_t length = 0;
	
	if (read(messaging->pipe_gtk[0], &length, sizeof(length)) < sizeof(length)) {
		return 0;
	}
	
	return length;
}

ssize_t CGTK_recv_gnunet_message(messaging_t* messaging, char* buffer, size_t length) {
	return read(messaging->pipe_gtk[0], buffer, length);
}
