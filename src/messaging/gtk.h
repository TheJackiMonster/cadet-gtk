//
// Created by thejackimonster on 20.05.20.
//

void CGTK_prepare_gtk(messaging_t* messaging);

void CGTK_send_gtk_identity(messaging_t* messaging, const struct GNUNET_PeerIdentity* identity);

void CGTK_send_gtk_found(messaging_t* messaging, const char* name, const struct GNUNET_PeerIdentity* identity);

void CGTK_send_gtk_connect(messaging_t* messaging, const struct GNUNET_PeerIdentity* source,
						   const struct GNUNET_HashCode* port);

void CGTK_send_gtk_disconnect(messaging_t* messaging, const struct GNUNET_PeerIdentity* source,
							  const struct GNUNET_HashCode* port);

ssize_t CGTK_send_gtk_message(messaging_t* messaging, const struct GNUNET_PeerIdentity* source,
							  const struct GNUNET_HashCode* port, const char* buffer, size_t length);

msg_type_t CGTK_recv_gtk_msg_type(messaging_t* messaging);

const struct GNUNET_HashCode* CGTK_recv_gtk_hashcode(messaging_t* messaging);

const struct GNUNET_PeerIdentity* CGTK_recv_gtk_identity(messaging_t* messaging);

size_t CGTK_recv_gtk_msg_length(messaging_t* messaging);

ssize_t CGTK_recv_gtk_message(messaging_t* messaging, char* buffer, size_t length);
