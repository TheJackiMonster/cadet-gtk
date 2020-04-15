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
	struct GNUNET_CONTAINER_MultiPeerMap* channel_map;
	struct GNUNET_SCHEDULER_Task* poll;
} session;

static int CGTK_clear_channel(void *cls, const struct GNUNET_PeerIdentity *key, void *value) {
	struct GNUNET_CADET_Channel* channel = (struct GNUNET_CADET_Channel*) value;
	
	GNUNET_CADET_channel_destroy(channel);
}

static void CGTK_shutdown(void* cls) {
	if (session.poll) {
		GNUNET_SCHEDULER_cancel(session.poll);
		session.poll = NULL;
	}
	
	if (session.channel_map) {
		GNUNET_CONTAINER_multipeermap_iterate(session.channel_map, CGTK_clear_channel, NULL);
		GNUNET_CONTAINER_multipeermap_destroy(session.channel_map);
		session.channel_map = NULL;
	}
	
	if (session.listen) {
		GNUNET_CADET_close_port(session.listen);
		session.listen = NULL;
	}
	
	GNUNET_CADET_disconnect(session.cadet);
	session.cadet = NULL;
	
	CGTK_close_messaging(messaging);
}

static int CGTK_add_new_channel(struct GNUNET_CADET_Channel* channel, const struct GNUNET_PeerIdentity* identity) {
	if (!session.channel_map) {
		session.channel_map = GNUNET_CONTAINER_multipeermap_create(8, GNUNET_NO);
	}
	
	return GNUNET_CONTAINER_multipeermap_put(
			session.channel_map,
			identity,
			channel,
			GNUNET_CONTAINER_MULTIHASHMAPOPTION_MULTIPLE
	);
}

static void* CGTK_on_connect(void* cls, struct GNUNET_CADET_Channel* channel, const struct GNUNET_PeerIdentity* source) {
	int res = CGTK_add_new_channel(channel, source);
	
	if (res == GNUNET_SYSERR) {
		GNUNET_CADET_channel_destroy(channel);
		return NULL;
	}
	
	CGTK_send_gtk_connect(messaging, source);
	
	return channel;
}


static void CGTK_on_disconnect(void* cls, const struct GNUNET_CADET_Channel* channel) {
	const union GNUNET_CADET_ChannelInfo* info = GNUNET_CADET_channel_get_info(
			channel, GNUNET_CADET_OPTION_PEER
	);
	
	CGTK_send_gtk_disconnect(messaging, &(info->peer));
	
	if (session.channel_map) {
		if (GNUNET_CONTAINER_multipeermap_remove(session.channel_map, &(info->peer), channel) == GNUNET_YES) {
			if (GNUNET_CONTAINER_multipeermap_size(session.channel_map) == 0) {
				GNUNET_CONTAINER_multipeermap_destroy(session.channel_map);
				session.channel_map = NULL;
			}
		}
	}
}

static void CGTK_on_window_size_change(void* cls, const struct GNUNET_CADET_Channel* channel, int window_size) {
	// TODO
}

static void CGTK_handle_message(const struct GNUNET_CADET_Channel* channel, const struct GNUNET_PeerIdentity* source,
		const struct GNUNET_MessageHeader* message) {
	size_t payload_size = ntohs(message->size) - sizeof(*message);
	uint16_t length = ntohs(message->size) - sizeof(*message);
	
	const char* buffer = (const char *) &message[1];
	
	GNUNET_CADET_receive_done(channel);
	
	ssize_t result = CGTK_send_gtk_message(messaging, source, buffer, length);
	
	if (result < 0) {
		GNUNET_SCHEDULER_shutdown();
	}
}

static int check_channel_message(void* cls, const struct GNUNET_MessageHeader* message) {
	const struct GNUNET_PeerIdentity* source = (struct GNUNET_PeerIdentity*) cls;
	
	return (session.channel_map) && (GNUNET_CONTAINER_multipeermap_contains(
		session.channel_map, source
	))? GNUNET_OK : GNUNET_NO;
}

static void handle_channel_message(void* cls, const struct GNUNET_MessageHeader* message) {
	const struct GNUNET_PeerIdentity* source = (struct GNUNET_PeerIdentity*) cls;
	
	CGTK_handle_message(
			GNUNET_CONTAINER_multipeermap_get(session.channel_map, source),
			source,
			message
	);
}

static int check_port_message(void* cls, const struct GNUNET_MessageHeader* message) {
	return GNUNET_OK;
}

static void handle_port_message(void* cls, const struct GNUNET_MessageHeader* message) {
	struct GNUNET_CADET_Channel* channel = (struct GNUNET_CADET_Channel*) cls;
	
	const union GNUNET_CADET_ChannelInfo* info = GNUNET_CADET_channel_get_info(
			channel, GNUNET_CADET_OPTION_PEER
	);
	
	CGTK_handle_message(channel, &(info->peer), message);
}

static void CGTK_poll(void* cls) {
	session.poll = NULL;
	
	msg_type_t type = CGTK_recv_gtk_msg_type(messaging);
	
	switch (type) {
		case MSG_GNUNET_PORT: {
			if (session.listen) {
				const struct GNUNET_HashCode* port = CGTK_recv_gtk_hashcode(messaging);
				
				if (!port) {
					GNUNET_SCHEDULER_shutdown();
					return;
				}
				
				GNUNET_CADET_close_port(session.listen);
				session.listen = NULL;
				
				struct GNUNET_MQ_MessageHandler handlers[] = {
						GNUNET_MQ_hd_var_size(
								port_message,
								GNUNET_MESSAGE_TYPE_CADET_CLI,
								struct GNUNET_MessageHeader,
								NULL
						), GNUNET_MQ_handler_end()
				};
				
				session.listen = GNUNET_CADET_open_port(
						session.cadet,
						port,
						&CGTK_on_connect,
						NULL,
						&CGTK_on_window_size_change,
						&CGTK_on_disconnect,
						handlers
				);
			}
			
			break;
		} case MSG_GNUNET_SEND_MESSAGE: {
			const struct GNUNET_PeerIdentity* destination = CGTK_recv_gtk_identity(messaging);
			struct GNUNET_CADET_Channel* channel = NULL;
			
			if (!destination) {
				GNUNET_SCHEDULER_shutdown();
				return;
			}
			
			const struct GNUNET_HashCode* port = CGTK_recv_gtk_hashcode(messaging);
			
			if (!port) {
				GNUNET_SCHEDULER_shutdown();
				return;
			}
			
			if ((!session.channel_map) || (!GNUNET_CONTAINER_multipeermap_contains(session.channel_map, destination))) {
				struct GNUNET_MQ_MessageHandler handlers[] = {
						GNUNET_MQ_hd_var_size(
								channel_message,
								GNUNET_MESSAGE_TYPE_CADET_CLI,
								struct GNUNET_MessageHeader,
								NULL
						), GNUNET_MQ_handler_end()
				};
				
				channel = GNUNET_CADET_channel_create(
						session.cadet,
						destination,
						destination,
						port,
						&CGTK_on_window_size_change,
						&CGTK_on_disconnect,
						handlers
				);
				
				int res = CGTK_add_new_channel(channel, destination);
				
				if (res == GNUNET_SYSERR) {
					GNUNET_CADET_channel_destroy(channel);
					channel = NULL;
				}
			} else {
				channel = (struct GNUNET_CADET_Channel *) GNUNET_CONTAINER_multipeermap_get(
						session.channel_map, destination
				);
			}
			
			size_t length = CGTK_recv_gtk_msg_length(messaging);
			char buffer[60000 + 1];
			
			if (length == 0) {
				break;
			}
			
			struct GNUNET_MessageHeader *msg;
			struct GNUNET_MQ_Envelope* env = NULL;
			
			if (channel) {
				env = GNUNET_MQ_msg_extra(msg, length, GNUNET_MESSAGE_TYPE_CADET_CLI);
			}
			
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
				
				if (env) {
					GNUNET_memcpy (&(msg[1]) + complete, buffer, offset);
				}
				
				complete += offset;
			}
			
			if (channel) {
				struct GNUNET_MQ_Handle* mq = GNUNET_CADET_get_mq(channel);
				
				GNUNET_MQ_send(mq, env);
				GNUNET_MQ_notify_sent(env, CGTK_poll, cls);
			}
			
			return;
		} case MSG_ERROR: {
			GNUNET_SCHEDULER_shutdown();
			return;
		} default: {
			break;
		}
	}
	
	if ((session.listen != NULL) || (session.channel_map != NULL)) {
		struct GNUNET_TIME_Relative delay = GNUNET_TIME_relative_get_second_();
		
		session.poll = GNUNET_SCHEDULER_add_delayed(delay, &CGTK_poll, NULL);
	}
}

void CGTK_run(void* cls, char*const* args, const char* cfgfile, const struct GNUNET_CONFIGURATION_Handle* cfg) {
	messaging = (messaging_t*) cls;
	
	session.cfg = cfg;
	session.cadet = GNUNET_CADET_connect(cfg);
	session.listen = NULL;
	session.channel_map = NULL;
	session.poll = NULL;
	
	if (!session.cadet) {
		GNUNET_SCHEDULER_shutdown();
		return;
	}
	
	GNUNET_SCHEDULER_add_shutdown(&CGTK_shutdown, NULL);
	
	struct GNUNET_HashCode port;
	GNUNET_CRYPTO_hash(NULL, 0, &port);
	
	struct GNUNET_MQ_MessageHandler handlers[] = {
			GNUNET_MQ_hd_var_size(
					port_message,
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
