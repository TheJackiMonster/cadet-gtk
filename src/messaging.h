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
	NONE = 0,
	
	GTK_IDENTITY = 1,
	GTK_PEERS = 2,
	GTK_CONNECT = 3,
	GTK_DISCONNECT = 4,
	GTK_SEND_COMPLETE = 5,
	GTK_RECV_MESSAGE = 6,
	
	GNUNET_PEERS = 20,
	GNUNET_SEND_MESSAGE = 50,
	GNUNET_POLL_MESSAGE = 60
} msg_type_t;

void* CGTK_on_connect(void* cls, struct GNUNET_CADET_Channel* channel, const struct GNUNET_PeerIdentity* source);

void CGTK_on_disconnect(void* cls, const struct GNUNET_CADET_Channel* channel);

void CGTK_on_window_size_change(void* cls, const struct GNUNET_CADET_Channel* channel, int window_size);

void CGTK_init_messaging(messaging_t* messaging);

void CGTK_prepare_gtk(messaging_t* messaging);

void CGTK_prepare_gnunet(messaging_t* messaging);

msg_type_t CGTK_recv_gtk_msg_type(messaging_t* messaging);

msg_type_t CGTK_recv_gnunet_msg_type(messaging_t* messaging);

void CGTK_send_gtk_identity(messaging_t* messaging, const struct GNUNET_PeerIdentity* identity);

const char* CGTK_recv_gnunet_identity(messaging_t* messaging);

void CGTK_close_messaging(messaging_t* messaging);

#endif //CADET_GTK_MESSAGING_H
