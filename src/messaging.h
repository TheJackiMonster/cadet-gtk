//
// Created by thejackimonster on 13.04.20.
//

#ifndef CADET_GTK_MESSAGING_H
#define CADET_GTK_MESSAGING_H

#include "gtk.h"
#include "gnunet.h"

/** @defgroup messaging_group Messaging
 *  Communication between the processes handling GNUnet and a GUI.
 *  @{
 */

/**
 * A struct which takes care of communication between processes of GNUnet and a GUI.
 *
 * @author TheJackiMonster
 * @since 13.04.20.
 */
typedef struct messaging_t {
	int pipe_gui [2];
	int pipe_gnunet [2];
} messaging_t;

/**
 * An enum to identify the type of a message received via a messaging_t struct.
 *
 * @author TheJackiMonster
 * @since 13.04.20
 */
typedef enum msg_type_t {
	MSG_NONE = 0,
	
	MSG_GUI_IDENTITY = 1,
	MSG_GUI_FOUND = 2,
	MSG_GUI_CONNECT = 3,
	MSG_GUI_DISCONNECT = 4,
	MSG_GUI_SEND_COMPLETE = 5,
	MSG_GUI_RECV_MESSAGE = 6,
	MSG_GUI_FILE_PROGRESS = 7,
	MSG_GUI_FILE_COMPLETE = 8,
	MSG_GUI_FILE_DELETE = 9,
	
	MSG_GNUNET_HOST = 10,
	MSG_GNUNET_SEARCH = 20,
	MSG_GNUNET_GROUP = 30,
	MSG_GNUNET_EXIT = 40,
	MSG_GNUNET_SEND_MESSAGE = 50,
	MSG_GNUNET_POLL_MESSAGE = 60,
	MSG_GNUNET_UPLOAD_FILE = 70,
	MSG_GNUNET_DOWNLOAD_FILE = 80,
	MSG_GNUNET_UNINDEX_FILE = 90,
	
	MSG_ERROR = -1
} msg_type_t;

/**
 * Initializes a messaging_t struct with two working pipes and
 * setups an acceleration structure for hash lookups.
 *
 * @warning exits on fail of creating pipes with EXIT_FAILURE
 *
 * @param messaging A pointer to a valid messaging_t struct (non-null)
 */
void CGTK_init_messaging(messaging_t* messaging);

#include "messaging/gui.h"
#include "messaging/gnunet.h"

/**
 * Securely closes all file descriptors to the pipes of the given
 * messaging_t struct.
 *
 * @param messaging A pointer to a valid messaging_t struct (non-null)
 */
void CGTK_close_messaging(messaging_t* messaging);

/**
 * Clears all left over space allocated for acceleration structures.
 */
void CGTK_shutdown_messaging(void);

/** @} */

#endif //CADET_GTK_MESSAGING_H
