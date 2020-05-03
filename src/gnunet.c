//
// Created by thejackimonster on 14.04.20.
//

#include "gnunet.h"

#include "config.h"
#include "messaging.h"

static messaging_t* messaging;

static struct {
	const struct GNUNET_CONFIGURATION_Handle* cfg;
	struct GNUNET_CADET_Handle* cadet;
	struct GNUNET_HashCode port;
	struct GNUNET_CADET_Port* listen;
	struct GNUNET_CONTAINER_MultiPeerMap* connections;
	struct GNUNET_SCHEDULER_Task* idle;
	struct GNUNET_TIME_Relative delay;
} session;

typedef struct {
	struct GNUNET_PeerIdentity identity;
	struct GNUNET_HashCode port;
	struct GNUNET_CADET_Channel* channel;
} connection_t;

static connection_t* CGTK_connection_create(const struct GNUNET_PeerIdentity* identity, const struct GNUNET_HashCode* port,
		struct GNUNET_CADET_Channel* channel) {
	connection_t* connection = GNUNET_malloc(sizeof(connection_t));
	
	GNUNET_memcpy(&(connection->identity), identity, sizeof(struct GNUNET_PeerIdentity));
	GNUNET_memcpy(&(connection->port), port, sizeof(struct GNUNET_HashCode));
	
	connection->channel = channel;
	
	return connection;
}

static void CGTK_connection_destroy(connection_t* connection, bool close_channel) {
	if ((close_channel) && (connection->channel)) {
		GNUNET_CADET_channel_destroy(connection->channel);
		connection->channel = NULL;
	}
	
	GNUNET_free(connection);
}

static int CGTK_clear_connection(void *cls, const struct GNUNET_PeerIdentity* key, void* value) {
	CGTK_connection_destroy((connection_t*) value, true);
	return GNUNET_YES;
}

static void CGTK_shutdown(void* cls) {
	if (session.idle) {
		GNUNET_SCHEDULER_cancel(session.idle);
		session.idle = NULL;
	}
	
	if (session.connections) {
		GNUNET_CONTAINER_multipeermap_iterate(session.connections, CGTK_clear_connection, NULL);
		GNUNET_CONTAINER_multipeermap_destroy(session.connections);
		session.connections = NULL;
	}
	
	if (session.listen) {
		GNUNET_CADET_close_port(session.listen);
		session.listen = NULL;
	}
	
	GNUNET_CADET_disconnect(session.cadet);
	session.cadet = NULL;
	
	CGTK_close_messaging(messaging);
}

static int CGTK_add_new_connection(connection_t* connection) {
	if (!session.connections) {
		session.connections = GNUNET_CONTAINER_multipeermap_create(8, GNUNET_NO);
	}
	
	return GNUNET_CONTAINER_multipeermap_put(
			session.connections,
			&(connection->identity),
			connection,
			GNUNET_CONTAINER_MULTIHASHMAPOPTION_MULTIPLE
	);
}

static int CGTK_remove_connection(connection_t* connection) {
	if (session.connections) {
		return GNUNET_CONTAINER_multipeermap_remove(session.connections, &(connection->identity), connection);
	} else {
		return GNUNET_NO;
	}
}

static void* CGTK_on_connect(void* cls, struct GNUNET_CADET_Channel* channel, const struct GNUNET_PeerIdentity* source) {
	connection_t* connection = CGTK_connection_create(source, &(session.port), channel);
	
	int res = CGTK_add_new_connection(connection);
	
	if (res == GNUNET_SYSERR) {
		CGTK_connection_destroy(connection, true);
		return NULL;
	}
	
	CGTK_send_gtk_connect(messaging, &(connection->identity), &(connection->port));
	
	return connection;
}

static void CGTK_on_disconnect(void* cls, const struct GNUNET_CADET_Channel* channel) {
	connection_t* connection = (connection_t*) cls;
	
	CGTK_send_gtk_disconnect(messaging, &(connection->identity), &(connection->port));
	CGTK_remove_connection(connection);
	
	CGTK_connection_destroy(connection, false);
}

static void CGTK_on_window_size_change(void* cls, const struct GNUNET_CADET_Channel* channel, int window_size) {
	// TODO
}

static void CGTK_handle_message(connection_t* connection, const struct GNUNET_MessageHeader* message) {
	uint16_t length = ntohs(message->size) - sizeof(*message);
	
	const char* buffer = (const char *) &message[1];
	
	GNUNET_CADET_receive_done(connection->channel);
	
	ssize_t result = CGTK_send_gtk_message(messaging, &(connection->identity), &(connection->port), buffer, length);
	
	if (result < 0) {
		GNUNET_SCHEDULER_shutdown();
	}
}

static int check_channel_message(void* cls, const struct GNUNET_MessageHeader* message) {
	connection_t* connection = (connection_t*) cls;
	
	return (session.connections) && (GNUNET_CONTAINER_multipeermap_contains(
		session.connections, &(connection->identity)
	))? GNUNET_OK : GNUNET_NO;
}

static void handle_channel_message(void* cls, const struct GNUNET_MessageHeader* message) {
	CGTK_handle_message((connection_t*) cls, message);
}

static int check_port_message(void* cls, const struct GNUNET_MessageHeader* message) {
	return GNUNET_OK;
}

static void handle_port_message(void* cls, const struct GNUNET_MessageHeader* message) {
	CGTK_handle_message((connection_t*) cls, message);
}

static void CGTK_idle(void* cls);

static int CGTK_send_message(void *cls, const struct GNUNET_PeerIdentity* key, void* value) {
	connection_t* connection = (connection_t*) value;
	
	if (GNUNET_CRYPTO_hash_cmp(&(connection->port), (struct GNUNET_HashCode*) cls) == 0) {
		size_t length = CGTK_recv_gtk_msg_length(messaging);
		char buffer[60000 + 1];
		
		if (length == 0) {
			memset(cls, 0, sizeof(struct GNUNET_HashCode));
			return GNUNET_NO;
		}
		
		struct GNUNET_MessageHeader *msg;
		struct GNUNET_MQ_Envelope* env = NULL;
		
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
					return GNUNET_NO;
				}
				
				offset += done;
			}
			
			if (env) {
				GNUNET_memcpy (&(msg[1]) + complete, buffer, offset);
			}
			
			complete += offset;
		}
		
		struct GNUNET_MQ_Handle* mq = GNUNET_CADET_get_mq(connection->channel);
		
		GNUNET_MQ_send(mq, env);
		GNUNET_MQ_notify_sent(env, CGTK_idle, cls);
		
		return GNUNET_NO;
	}
	
	return GNUNET_YES;
}

static int CGTK_exit_connection(void *cls, const struct GNUNET_PeerIdentity* key, void* value) {
	connection_t* connection = (connection_t*) value;
	
	if (GNUNET_CRYPTO_hash_cmp(&(connection->port), (struct GNUNET_HashCode*) cls) == 0) {
		CGTK_remove_connection(connection);
		
		CGTK_connection_destroy(connection, true);
		return GNUNET_NO;
	}
	
	return GNUNET_YES;
}

static void CGTK_idle(void* cls) {
	session.idle = NULL;
	
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
				
				GNUNET_memcpy(&(session.port), port, sizeof(struct GNUNET_HashCode));
				
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
						&(session.port),
						&CGTK_on_connect,
						NULL,
						&CGTK_on_window_size_change,
						&CGTK_on_disconnect,
						handlers
				);
			}
			
			break;
		} case MSG_GNUNET_SEND_MESSAGE: {
			const struct GNUNET_PeerIdentity *destination = CGTK_recv_gtk_identity(messaging);
			
			if (!destination) {
				GNUNET_SCHEDULER_shutdown();
				return;
			}
			
			const struct GNUNET_HashCode *port = CGTK_recv_gtk_hashcode(messaging);
			
			if (!port) {
				GNUNET_SCHEDULER_shutdown();
				return;
			}
			
			if ((!session.connections) || (!GNUNET_CONTAINER_multipeermap_contains(session.connections, destination))) {
				struct GNUNET_MQ_MessageHandler handlers[] = {
						GNUNET_MQ_hd_var_size(
								channel_message,
								GNUNET_MESSAGE_TYPE_CADET_CLI,
								struct GNUNET_MessageHeader,
								NULL
						), GNUNET_MQ_handler_end()
				};
				
				connection_t *connection = CGTK_connection_create(destination, port, NULL);
				
				connection->channel = GNUNET_CADET_channel_create(
						session.cadet,
						connection,
						destination,
						port,
						&CGTK_on_window_size_change,
						&CGTK_on_disconnect,
						handlers
				);
				
				if (CGTK_add_new_connection(connection) == GNUNET_SYSERR) {
					CGTK_connection_destroy(connection, true);
					connection = NULL;
				}
			}
			
			struct GNUNET_HashCode hashcode;
			GNUNET_memcpy(&hashcode, port, sizeof(struct GNUNET_HashCode));
			
			GNUNET_CONTAINER_multipeermap_get_multiple(session.connections, destination, CGTK_send_message, &hashcode);
			
			if (GNUNET_CRYPTO_hash_cmp(port, &hashcode)) {
				break;
			}
			
			return;
		} case MSG_GNUNET_EXIT: {
			const struct GNUNET_PeerIdentity *destination = CGTK_recv_gtk_identity(messaging);
			
			if (!destination) {
				GNUNET_SCHEDULER_shutdown();
				return;
			}
			
			const struct GNUNET_HashCode *port = CGTK_recv_gtk_hashcode(messaging);
			
			if (!port) {
				GNUNET_SCHEDULER_shutdown();
				return;
			}
			
			if (session.connections) {
				struct GNUNET_HashCode hashcode;
				GNUNET_memcpy(&hashcode, port, sizeof(struct GNUNET_HashCode));
				
				GNUNET_CONTAINER_multipeermap_get_multiple(session.connections, destination, CGTK_exit_connection, &hashcode);
			}
			
			break;
		} case MSG_ERROR: {
			GNUNET_SCHEDULER_shutdown();
			return;
		} default: {
			break;
		}
	}
	
	if ((session.listen != NULL) || (session.connections != NULL)) {
		session.idle = GNUNET_SCHEDULER_add_delayed_with_priority(
				session.delay,
				GNUNET_SCHEDULER_PRIORITY_IDLE,
				&CGTK_idle,
				NULL
		);
	}
}

void CGTK_run(void* cls, char*const* args, const char* cfgfile, const struct GNUNET_CONFIGURATION_Handle* cfg) {
	messaging = (messaging_t*) cls;
	
	session.cfg = cfg;
	session.cadet = GNUNET_CADET_connect(cfg);
	memset(&(session.port), 0, sizeof(struct GNUNET_HashCode));
	session.listen = NULL;
	session.connections = NULL;
	session.idle = NULL;
	
	if (!session.cadet) {
		GNUNET_SCHEDULER_shutdown();
		return;
	}
	
	GNUNET_SCHEDULER_add_shutdown(&CGTK_shutdown, NULL);
	
	GNUNET_CRYPTO_hash(NULL, 0, &(session.port));
	
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
			&(session.port),
			&CGTK_on_connect,
			NULL,
			&CGTK_on_window_size_change,
			&CGTK_on_disconnect,
			handlers
	);
	
	session.delay = GNUNET_TIME_relative_multiply(
			GNUNET_TIME_UNIT_MILLISECONDS,
			CGTK_GNUNET_SESSION_IDLE_DELAY_MS
	);
	
	session.idle = GNUNET_SCHEDULER_add_delayed_with_priority(
			session.delay,
			GNUNET_SCHEDULER_PRIORITY_IDLE,
			&CGTK_idle,
			NULL
	);
}
