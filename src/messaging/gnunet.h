//
// Created by thejackimonster on 20.05.20.
//

#ifndef CADET_GTK_MESSAGING_GNUNET_H
#define CADET_GTK_MESSAGING_GNUNET_H

void CGTK_prepare_gnunet(messaging_t* messaging);

ssize_t CGTK_send_gnunet_host(messaging_t* messaging, uint8_t visibility, const char* port, const char* name_regex);

ssize_t CGTK_send_gnunet_search(messaging_t* messaging, const char* name);

ssize_t CGTK_send_gnunet_message(messaging_t* messaging, const char* destination, const char* port, const char* buffer, size_t length);

void CGTK_send_gnunet_group(messaging_t* messaging, const char* port);

void CGTK_send_gnunet_exit(messaging_t* messaging, const char* destination, const char* port);

msg_type_t CGTK_recv_gnunet_msg_type(messaging_t* messaging);

const char* CGTK_recv_gnunet_identity(messaging_t* messaging);

const char* CGTK_recv_gnunet_port(messaging_t* messaging);

guint CGTK_recv_gnunet_hash(messaging_t* messaging);

size_t CGTK_recv_gnunet_msg_length(messaging_t* messaging);

ssize_t CGTK_recv_gnunet_message(messaging_t* messaging, char* buffer, size_t length);

#endif //CADET_GTK_MESSAGING_GNUNET_H
