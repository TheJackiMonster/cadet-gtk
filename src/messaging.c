//
// Created by thejackimonster on 13.04.20.
//

#include "messaging.h"

#include <unistd.h>
#include <fcntl.h>

void* CGTK_on_connect(void* cls, struct GNUNET_CADET_Channel* channel, const struct GNUNET_PeerIdentity* source) {
	// TODO
	
	printf("connect: %s\n", (const char*) source->public_key.q_y);
	
	return NULL;
}


void CGTK_on_disconnect(void* cls, const struct GNUNET_CADET_Channel* channel) {
	// TODO
	
	printf("disconnect!\n");
}

void CGTK_on_window_size_change(void* cls, const struct GNUNET_CADET_Channel* channel, int window_size) {
	// TODO
	
	printf("window_size_change: %d\n", window_size);
}

void CGTK_init_messaging(messaging_t* messaging) {
	if ((pipe(messaging->pipe_gnunet) == -1) ||
		(pipe2(messaging->pipe_gtk, O_NONBLOCK))) {
		exit(EXIT_FAILURE);
	}
}

static void close_if_open(int fd) {
	if (fd) {
		close(fd);
	}
}

void CGTK_prepare_gtk(messaging_t* messaging) {
	close_if_open(messaging->pipe_gnunet[0]);
	close_if_open(messaging->pipe_gtk[1]);
}

void CGTK_prepare_gnunet(messaging_t* messaging) {
	close_if_open(messaging->pipe_gnunet[1]);
	close_if_open(messaging->pipe_gtk[0]);
}

msg_type_t CGTK_recv_gtk_msg_type(messaging_t* messaging) {
	msg_type_t type = NONE;
	
	read(messaging->pipe_gnunet[0], &type, sizeof(type));
	
	return type;
}

msg_type_t CGTK_recv_gnunet_msg_type(messaging_t* messaging) {
	msg_type_t type = NONE;
	
	if (read(messaging->pipe_gtk[0], &type, sizeof(type)) < sizeof(type)) {
		return NONE;
	}
	
	return type;
}

void CGTK_send_gtk_identity(messaging_t* messaging, const struct GNUNET_PeerIdentity* identity) {
	msg_type_t type = GTK_IDENTITY;
	
	write(messaging->pipe_gtk[1], &type, sizeof(type));
	write(messaging->pipe_gtk[1], identity, sizeof(struct GNUNET_PeerIdentity));
}

const char* CGTK_recv_gnunet_identity(messaging_t* messaging) {
	struct GNUNET_PeerIdentity identity;
	
	read(messaging->pipe_gtk[0], &identity, sizeof(identity));
	
	return GNUNET_i2s_full(&identity);
}

void CGTK_close_messaging(messaging_t* messaging) {
	close_if_open(messaging->pipe_gnunet[0]);
	close_if_open(messaging->pipe_gnunet[1]);
	close_if_open(messaging->pipe_gtk[0]);
	close_if_open(messaging->pipe_gtk[1]);
}
