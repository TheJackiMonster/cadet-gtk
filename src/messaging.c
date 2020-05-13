//
// Created by thejackimonster on 13.04.20.
//

#include "messaging.h"

#include <unistd.h>
#include <fcntl.h>

static struct GNUNET_CONTAINER_MultiHashMap* port_lookup;

void CGTK_init_messaging(messaging_t* messaging) {
	if ((pipe2(messaging->pipe_gnunet, O_NONBLOCK) == -1) ||
		(pipe2(messaging->pipe_gtk, O_NONBLOCK) == -1)) {
		exit(EXIT_FAILURE);
	}
	
	port_lookup = GNUNET_CONTAINER_multihashmap_create(8, GNUNET_NO);
	
	char* empty_string = GNUNET_malloc(sizeof(char));
	empty_string[0] = '\0';
	
	struct GNUNET_HashCode hashcode;
	GNUNET_CRYPTO_hash(NULL, 0, &hashcode);
	GNUNET_CONTAINER_multihashmap_put(
			port_lookup,
			&hashcode,
			empty_string,
			GNUNET_CONTAINER_MULTIHASHMAPOPTION_UNIQUE_ONLY
	);
}

static void close_if_open(int* fd) {
	if (*fd) {
		close(*fd);
		*fd = 0;
	}
}

void CGTK_prepare_gtk(messaging_t* messaging) {
	close_if_open(&(messaging->pipe_gnunet[0]));
	close_if_open(&(messaging->pipe_gtk[1]));
}

void CGTK_prepare_gnunet(messaging_t* messaging) {
	close_if_open(&(messaging->pipe_gnunet[1]));
	close_if_open(&(messaging->pipe_gtk[0]));
}

msg_type_t CGTK_recv_gtk_msg_type(messaging_t* messaging) {
	msg_type_t type = MSG_NONE;
	
	if (read(messaging->pipe_gnunet[0], &type, sizeof(type)) < sizeof(type)) {
		return MSG_ERROR;
	}
	
	return type;
}

msg_type_t CGTK_recv_gnunet_msg_type(messaging_t* messaging) {
	msg_type_t type = MSG_NONE;
	
	if (read(messaging->pipe_gtk[0], &type, sizeof(type)) < sizeof(type)) {
		return MSG_ERROR;
	}
	
	return type;
}

void CGTK_send_gtk_identity(messaging_t* messaging, const struct GNUNET_PeerIdentity* identity) {
	msg_type_t type = MSG_GTK_IDENTITY;
	
	write(messaging->pipe_gtk[1], &type, sizeof(type));
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

void CGTK_send_gnunet_port(messaging_t* messaging, const char* port) {
	size_t port_len = strlen(port);
	
	struct GNUNET_HashCode hashcode;
	GNUNET_CRYPTO_hash(port, port_len, &hashcode);
	
	CGTK_add_port_to_lookup(port, port_len, &hashcode);
	
	msg_type_t type = MSG_GNUNET_PORT;
	
	write(messaging->pipe_gnunet[1], &type, sizeof(type));
	write(messaging->pipe_gnunet[1], &hashcode, sizeof(hashcode));
}

ssize_t CGTK_send_gnunet_message(messaging_t* messaging, const char* destination, const char* port,
		const char* buffer, size_t length) {
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
	size_t port_len = strlen(port);
	
	struct GNUNET_HashCode hashcode;
	GNUNET_CRYPTO_hash(port, port_len, &hashcode);
	
	CGTK_add_port_to_lookup(port, port_len, &hashcode);
	
	msg_type_t type = MSG_GNUNET_GROUP;
	
	write(messaging->pipe_gnunet[1], &type, sizeof(type));
	write(messaging->pipe_gnunet[1], &hashcode, sizeof(hashcode));
}

void CGTK_send_gnunet_exit(messaging_t* messaging, const char* destination, const char* port) {
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

void CGTK_close_messaging(messaging_t* messaging) {
	close_if_open(&(messaging->pipe_gnunet[0]));
	close_if_open(&(messaging->pipe_gnunet[1]));
	close_if_open(&(messaging->pipe_gtk[0]));
	close_if_open(&(messaging->pipe_gtk[1]));
}

static int CGTK_clear_lookup(void *cls, const struct GNUNET_HashCode* key, void* value) {
	GNUNET_free(value);
	return GNUNET_YES;
}

void CGTK_shutdown_messaging(void) {
	if (port_lookup) {
		GNUNET_CONTAINER_multihashmap_iterate(port_lookup, CGTK_clear_lookup, NULL);
		GNUNET_CONTAINER_multihashmap_destroy(port_lookup);
		port_lookup = NULL;
	}
}
