//
// Created by thejackimonster on 13.04.20.
//

#ifndef CADET_GTK_MESSAGING_H
#define CADET_GTK_MESSAGING_H

#include <gtk/gtk.h>
#include <gnunet/gnunet_config.h>
#include <gnunet/gnunet_cadet_service.h>
#include <gnunet/gnunet_regex_service.h>
#include <gnunet/gnunet_protocols.h>

typedef struct messaging_t {
	int pipe_gtk [2];
	int pipe_gnunet [2];
} messaging_t;

typedef enum {
	MSG_NONE = 0,
	
	MSG_GTK_IDENTITY = 1,
	MSG_GTK_FOUND = 2,
	MSG_GTK_CONNECT = 3,
	MSG_GTK_DISCONNECT = 4,
	MSG_GTK_SEND_COMPLETE = 5,
	MSG_GTK_RECV_MESSAGE = 6,
	
	MSG_GNUNET_HOST = 10,
	MSG_GNUNET_SEARCH = 20,
	MSG_GNUNET_GROUP = 30,
	MSG_GNUNET_EXIT = 40,
	MSG_GNUNET_SEND_MESSAGE = 50,
	MSG_GNUNET_POLL_MESSAGE = 60,
	
	MSG_ERROR = -1
} msg_type_t;

void CGTK_init_messaging(messaging_t* messaging);

#include "messaging/gtk.h"
#include "messaging/gnunet.h"

void CGTK_close_messaging(messaging_t* messaging);

void CGTK_shutdown_messaging(void);

#endif //CADET_GTK_MESSAGING_H
