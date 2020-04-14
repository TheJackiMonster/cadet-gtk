//
// Created by thejackimonster on 14.04.20.
//

#include "gnunet.h"

#include "messaging.h"

static messaging_t* messaging;

static struct {
	const struct GNUNET_CONFIGURATION_Handle* cfg;
	struct GNUNET_CADET_Handle* cadet;
	struct GNUNET_CADET_Port* listen;
	struct GNUNET_CADET_Channel* channel;
	struct GNUNET_SCHEDULER_Task* poll;
} session;

static void CGTK_shutdown(void* cls) {
	if (session.poll) {
		GNUNET_SCHEDULER_cancel(session.poll);
		session.poll = NULL;
	}
	
	if (session.channel) {
		GNUNET_CADET_channel_destroy(session.channel);
		session.channel = NULL;
	}
	
	if (session.listen) {
		GNUNET_CADET_close_port(session.listen);
		session.listen = NULL;
	}
	
	GNUNET_CADET_disconnect(session.cadet);
	session.cadet = NULL;
	
	CGTK_close_messaging(messaging);
}

static void* CGTK_on_connect(void* cls, struct GNUNET_CADET_Channel* channel, const struct GNUNET_PeerIdentity* source) {
	CGTK_send_gtk_connect(messaging, source);
	
	GNUNET_CADET_close_port(session.listen);
	session.listen = NULL;
	
	session.channel = channel;
	
	return channel;
}


static void CGTK_on_disconnect(void* cls, const struct GNUNET_CADET_Channel* channel) {
	const union GNUNET_CADET_ChannelInfo* info = GNUNET_CADET_channel_get_info(
			channel, GNUNET_CADET_OPTION_PEER
	);
	
	CGTK_send_gtk_disconnect(messaging, &(info->peer));
	
	if (session.channel == channel) {
		session.channel = NULL;
	}
}

static void CGTK_on_window_size_change(void* cls, const struct GNUNET_CADET_Channel* channel, int window_size) {
	// TODO
}

static int check_message(void* cls, const struct GNUNET_MessageHeader* message) {
	return GNUNET_OK;
}

static void handle_message(void* cls, const struct GNUNET_MessageHeader* message) {
	size_t payload_size = ntohs(message->size) - sizeof(*message);
	uint16_t length = ntohs(message->size) - sizeof(*message);
	
	const char* buffer = (const char *) &message[1];
	
	const union GNUNET_CADET_ChannelInfo* info = GNUNET_CADET_channel_get_info(
			session.channel, GNUNET_CADET_OPTION_PEER
	);
	
	GNUNET_CADET_receive_done(session.channel);
	
	ssize_t result = CGTK_send_gtk_message(messaging, &(info->peer), buffer, length);
	
	if (result < 0) {
		GNUNET_SCHEDULER_shutdown();
	}
}

static void CGTK_poll(void* cls) {
	session.poll = NULL;
	
	msg_type_t type = CGTK_recv_gtk_msg_type(messaging);
	
	switch (type) {
		case MSG_GNUNET_SEND_MESSAGE: {
			struct GNUNET_PeerIdentity* destination = CGTK_recv_gtk_identity(messaging);
			
			if (destination == NULL) {
				GNUNET_SCHEDULER_shutdown();
				return;
			}
			
			size_t length = CGTK_recv_gtk_msg_length(messaging);
			char buffer[60000 + 1];
			
			if (length == 0) {
				break;
			}
			
			if (!session.channel) {
				GNUNET_SCHEDULER_shutdown();
				return;
			}
			
			struct GNUNET_MQ_Handle* mq = GNUNET_CADET_get_mq(session.channel);
			struct GNUNET_MessageHeader *msg;
			struct GNUNET_MQ_Envelope* env;
			
			env = GNUNET_MQ_msg_extra(msg, length, GNUNET_MESSAGE_TYPE_CADET_CLI);
			
			size_t complete = 0;
			
			while (complete < length) {
				ssize_t offset = 0;
				size_t remaining = length - complete;
				
				if (remaining > 60000) {
					remaining = 60000;
				}
				
				while (offset < remaining) {
					ssize_t done = CGTK_recv_gtk_message(messaging, buffer + offset, remaining - offset);
					
					if (done <= 0) {
						GNUNET_SCHEDULER_shutdown();
						return;
					}
					
					offset += done;
				}
				
				GNUNET_memcpy (&(msg[1]) + complete, buffer, offset);
				
				complete += offset;
			}
			
			GNUNET_MQ_send(mq, env);
			GNUNET_MQ_notify_sent(env, CGTK_poll, cls);
			return;
		} case MSG_ERROR: {
			GNUNET_SCHEDULER_shutdown();
			return;
		} default: {
			break;
		}
	}
	
	if ((session.listen != NULL) || (session.channel != NULL)) {
		struct GNUNET_TIME_Relative delay = GNUNET_TIME_relative_get_second_();
		
		session.poll = GNUNET_SCHEDULER_add_delayed(delay, &CGTK_poll, NULL);
	}
}

void CGTK_run(void* cls, char*const* args, const char* cfgfile, const struct GNUNET_CONFIGURATION_Handle* cfg) {
	messaging = (messaging_t*) cls;
	
	session.cfg = cfg;
	session.cadet = GNUNET_CADET_connect(cfg);
	session.listen = NULL;
	session.channel = NULL;
	session.poll = NULL;
	
	if (!session.cadet) {
		GNUNET_SCHEDULER_shutdown();
		return;
	}
	
	GNUNET_SCHEDULER_add_shutdown(&CGTK_shutdown, NULL);
	
	const char* port_s = "test\0";
	const char* port_us = GNUNET_STRINGS_to_utf8(port_s, strlen(port_s), "ASCII");
	
	struct GNUNET_HashCode port;
	GNUNET_CRYPTO_hash(port_us, strlen(port_us), &port);
	
	struct GNUNET_MQ_MessageHandler handlers[] = {
			GNUNET_MQ_hd_var_size(
					message,
					GNUNET_MESSAGE_TYPE_CADET_CLI,
			struct GNUNET_MessageHeader,
			NULL
			), GNUNET_MQ_handler_end()
	};
	
	struct GNUNET_PeerIdentity peer;
	
	if (GNUNET_CRYPTO_get_peer_identity(cfg, &peer) != GNUNET_OK) {
		memset(&peer, 0, sizeof(peer));
	}
	
	CGTK_send_gtk_identity(messaging, &peer);
	
	session.listen = GNUNET_CADET_open_port(
			session.cadet,
			&port,
			&CGTK_on_connect,
			NULL,
			&CGTK_on_window_size_change,
			&CGTK_on_disconnect,
			handlers
	);
	
	struct GNUNET_TIME_Relative delay = GNUNET_TIME_relative_get_second_();
	
	session.poll = GNUNET_SCHEDULER_add_delayed(delay, &CGTK_poll, NULL);
}
