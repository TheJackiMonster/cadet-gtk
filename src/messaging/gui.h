//
// Created by thejackimonster on 20.05.20.
//

#ifndef CADET_GTK_MESSAGING_GUI_H
#define CADET_GTK_MESSAGING_GUI_H

/** @addtogroup messaging_group
 *  @{
 */

/**
 * Prepares a messaging_t struct to receive and send messages from and to a GUI process.
 *
 * @param messaging A pointer to a valid messaging_t struct (non-null)
 */
void CGTK_prepare_gui(messaging_t* messaging);

void CGTK_send_gui_identity(messaging_t* messaging, const struct GNUNET_PeerIdentity* identity);

void CGTK_send_gui_found(messaging_t* messaging, const char* name, const struct GNUNET_PeerIdentity* identity);

void CGTK_send_gui_connect(messaging_t* messaging, const struct GNUNET_PeerIdentity* source,
						   const struct GNUNET_HashCode* port);

void CGTK_send_gui_disconnect(messaging_t* messaging, const struct GNUNET_PeerIdentity* source,
							  const struct GNUNET_HashCode* port);

ssize_t CGTK_send_gui_message(messaging_t* messaging, const struct GNUNET_PeerIdentity* source,
							  const struct GNUNET_HashCode* port, const char* buffer, size_t length);

ssize_t CGTK_send_gui_file_progress(messaging_t* messaging, const struct GNUNET_PeerIdentity* destination,
							 const struct GNUNET_HashCode* port, float progress, const char* path, const struct GNUNET_FS_Uri* uri);

ssize_t CGTK_send_gui_file_complete(messaging_t* messaging, const struct GNUNET_PeerIdentity* destination,
									const struct GNUNET_HashCode* port, const char* path, const struct GNUNET_FS_Uri* uri);

msg_type_t CGTK_recv_gui_msg_type(messaging_t* messaging);

uint8_t CGTK_recv_gui_code(messaging_t* messaging);

const struct GNUNET_HashCode* CGTK_recv_gui_hashcode(messaging_t* messaging);

const struct GNUNET_PeerIdentity* CGTK_recv_gui_identity(messaging_t* messaging);

size_t CGTK_recv_gui_msg_length(messaging_t* messaging);

ssize_t CGTK_recv_gui_message(messaging_t* messaging, char* buffer, size_t length);

const char* CGTK_recv_gui_path(messaging_t* messaging);

struct GNUNET_FS_Uri* CGTK_recv_gui_uri(messaging_t* messaging);

/** } */

#endif //CADET_GTK_MESSAGING_GUI_H
