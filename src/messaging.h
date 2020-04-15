//
// Created by thejackimonster on 13.04.20.
//

#ifndef CADET_GTK_MESSAGING_H
#define CADET_GTK_MESSAGING_H

#include <gnunet/gnunet_config.h>
#include <gnunet/gnunet_cadet_service.h>
#include <gnunet/gnunet_protocols.h>

typedef struct messaging_t {
	int pipe_gtk [2];
	int pipe_gnunet [2];
} messaging_t;

typedef enum {
	MSG_NONE = 0,
	
	MSG_GTK_IDENTITY = 1,
	MSG_GTK_PEERS = 2,
	MSG_GTK_CONNECT = 3,
	MSG_GTK_DISCONNECT = 4,
	MSG_GTK_SEND_COMPLETE = 5,
	MSG_GTK_RECV_MESSAGE = 6,
	
	MSG_GNUNET_PORT = 10,
	MSG_GNUNET_PEERS = 20,
	MSG_GNUNET_SEND_MESSAGE = 50,
	MSG_GNUNET_POLL_MESSAGE = 60,
	
	MSG_ERROR = -1
} msg_type_t;

void CGTK_init_messaging(messaging_t* messaging);

void CGTK_prepare_gtk(messaging_t* messaging);

void CGTK_prepare_gnunet(messaging_t* messaging);

msg_type_t CGTK_recv_gtk_msg_type(messaging_t* messaging);

msg_type_t CGTK_recv_gnunet_msg_type(messaging_t* messaging);

void CGTK_send_gtk_identity(messaging_t* messaging, const struct GNUNET_PeerIdentity* identity);

void CGTK_send_gtk_connect(messaging_t* messaging, const struct GNUNET_PeerIdentity* source);

void CGTK_send_gtk_disconnect(messaging_t* messaging, const struct GNUNET_PeerIdentity* source);

ssize_t CGTK_send_gtk_message(messaging_t* messaging, const struct GNUNET_PeerIdentity* source,
		const char* buffer, size_t length);

const char* CGTK_recv_gnunet_identity(messaging_t* messaging);

size_t CGTK_recv_gnunet_msg_length(messaging_t* messaging);

ssize_t CGTK_recv_gnunet_message(messaging_t* messaging, char* buffer, size_t length);

void CGTK_send_gnunet_port(messaging_t* messaging, const char* port);

ssize_t CGTK_send_gnunet_message(messaging_t* messaging, const char* destination, const char* port, const char* buffer, size_t length);

const struct GNUNET_HashCode* CGTK_recv_gtk_hashcode(messaging_t* messaging);

const struct GNUNET_PeerIdentity* CGTK_recv_gtk_identity(messaging_t* messaging);

size_t CGTK_recv_gtk_msg_length(messaging_t* messaging);

ssize_t CGTK_recv_gtk_message(messaging_t* messaging, char* buffer, size_t length);

void CGTK_close_messaging(messaging_t* messaging);

#endif //CADET_GTK_MESSAGING_H
