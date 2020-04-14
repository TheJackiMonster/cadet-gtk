//
// Created by thejackimonster on 13.04.20.
//

#include "messaging.h"

#include <unistd.h>
#include <fcntl.h>

void CGTK_init_messaging(messaging_t* messaging) {
	if ((pipe2(messaging->pipe_gnunet, O_NONBLOCK) == -1) ||
		(pipe2(messaging->pipe_gtk, O_NONBLOCK) == -1)) {
		exit(EXIT_FAILURE);
	}
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

void CGTK_send_gtk_connect(messaging_t* messaging, const struct GNUNET_PeerIdentity* source) {
	msg_type_t type = MSG_GTK_CONNECT;
	
	write(messaging->pipe_gtk[1], &type, sizeof(type));
	write(messaging->pipe_gtk[1], source, sizeof(struct GNUNET_PeerIdentity));
}

void CGTK_send_gtk_disconnect(messaging_t* messaging, const struct GNUNET_PeerIdentity* source) {
	msg_type_t type = MSG_GTK_DISCONNECT;
	
	write(messaging->pipe_gtk[1], &type, sizeof(type));
	write(messaging->pipe_gtk[1], source, sizeof(struct GNUNET_PeerIdentity));
}

ssize_t CGTK_send_gtk_message(messaging_t* messaging, const struct GNUNET_PeerIdentity* source,
		const char* buffer, size_t length) {
	msg_type_t type = MSG_GTK_RECV_MESSAGE;
	
	write(messaging->pipe_gtk[1], &type, sizeof(type));
	write(messaging->pipe_gtk[1], source, sizeof(struct GNUNET_PeerIdentity));
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

const char* CGTK_recv_gnunet_hashcode(messaging_t* messaging) {
	struct GNUNET_HashCode hashcode;
	
	if (read(messaging->pipe_gtk[0], &hashcode, sizeof(hashcode)) < sizeof(hashcode)) {
		return NULL;
	}
	
	return GNUNET_h2s_full(&hashcode);
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

ssize_t CGTK_send_gnunet_message(messaging_t* messaging, const char* destination, const char* buffer, size_t length) {
	struct GNUNET_PeerIdentity identity;
	
	if (GNUNET_CRYPTO_eddsa_public_key_from_string(destination, strlen(destination), &(identity.public_key)) != GNUNET_OK) {
		return -1;
	}
	
	msg_type_t type = MSG_GNUNET_SEND_MESSAGE;
	
	write(messaging->pipe_gnunet[1], &type, sizeof(type));
	write(messaging->pipe_gnunet[1], &identity, sizeof(identity));
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
