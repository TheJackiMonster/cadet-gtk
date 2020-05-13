//
// Created by thejackimonster on 14.04.20.
//

#include "gnunet.h"

#include "config.h"
#include "json.h"
#include "messaging.h"

static messaging_t* messaging;

static struct {
	const struct GNUNET_CONFIGURATION_Handle* cfg;
	struct GNUNET_CADET_Handle* cadet;
	
	struct GNUNET_HashCode port;
	struct GNUNET_CADET_Port* listen;
	
	struct GNUNET_CONTAINER_MultiPeerMap* connections;
	struct GNUNET_CONTAINER_MultiHashMap* group_ports;
	struct GNUNET_CONTAINER_MultiHashMap* groups;
	
	struct GNUNET_SCHEDULER_Task* idle;
	struct GNUNET_TIME_Relative delay;
} session;

typedef struct {
	struct GNUNET_PeerIdentity identity;
	struct GNUNET_HashCode port;
	struct GNUNET_CADET_Channel* channel;
	
	struct GNUNET_HashCode* group;
	char name [GNUNET_CRYPTO_PKEY_ASCII_LENGTH + 1];
} connection_t;

typedef struct {
	struct GNUNET_CADET_Port* listen;
	struct GNUNET_HashCode port;
} group_t;

typedef struct {
	const connection_t* sender;
	const char* content;
	size_t content_length;
} group_message_t;

static void CGTK_fatal_error(const char* error_message) {
	GNUNET_SCHEDULER_shutdown();
	
	if (error_message) {
		perror(error_message);
	}
}

static connection_t* CGTK_connection_create(const struct GNUNET_PeerIdentity* identity, const struct GNUNET_HashCode* port,
		struct GNUNET_CADET_Channel* channel) {
	connection_t* connection = GNUNET_malloc(sizeof(connection_t));
	
	GNUNET_memcpy(&(connection->identity), identity, sizeof(struct GNUNET_PeerIdentity));
	GNUNET_memcpy(&(connection->port), port, sizeof(struct GNUNET_HashCode));
	
	connection->channel = channel;
	connection->group = NULL;
	
	const char* identity_str = GNUNET_i2s_full(identity);
	
	strncpy(connection->name, identity_str, GNUNET_CRYPTO_PKEY_ASCII_LENGTH);
	connection->name[GNUNET_CRYPTO_PKEY_ASCII_LENGTH] = '\0';
	
	return connection;
}

static void CGTK_connection_destroy(connection_t* connection, bool close_channel) {
	if ((close_channel) && (connection->channel)) {
		GNUNET_CADET_channel_destroy(connection->channel);
		connection->channel = NULL;
	}
	
	GNUNET_free(connection);
}

static void CGTK_group_destroy(group_t* group);

static int CGTK_clear_connection(void *cls, const struct GNUNET_PeerIdentity* identity, void* value) {
	CGTK_connection_destroy((connection_t*) value, true);
	return GNUNET_YES;
}

static int CGTK_group_clear_connection(void* cls, const struct GNUNET_HashCode* group, void* value) {
	CGTK_connection_destroy((connection_t*) value, true);
	return GNUNET_YES;
}

static int CGTK_group_clear_port(void* cls, const struct GNUNET_HashCode* group, void* value) {
	CGTK_group_destroy((group_t*) value);
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
	
	if (session.groups) {
		GNUNET_CONTAINER_multihashmap_iterate(session.groups, CGTK_group_clear_connection, NULL);
		GNUNET_CONTAINER_multihashmap_destroy(session.groups);
		session.groups = NULL;
	}
	
	if (session.group_ports) {
		GNUNET_CONTAINER_multihashmap_iterate(session.group_ports, CGTK_group_clear_port, NULL);
		GNUNET_CONTAINER_multihashmap_destroy(session.group_ports);
		session.group_ports = NULL;
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

static int CGTK_group_add_new_connection(connection_t* connection) {
	if (!session.groups) {
		session.groups = GNUNET_CONTAINER_multihashmap_create(4, GNUNET_NO);
	}
	
	connection->group = &(connection->port);
	
	return GNUNET_CONTAINER_multihashmap_put(
			session.groups,
			connection->group,
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

static int CGTK_group_remove_connection(connection_t* connection) {
	if (session.groups) {
		return GNUNET_CONTAINER_multihashmap_remove(session.groups, connection->group, connection);
	} else {
		return GNUNET_NO;
	}
}

static void CGTK_idle(void* cls);

static int CGTK_group_push_message(void * cls, const struct GNUNET_HashCode* group, void* value) {
	const connection_t* connection = (const connection_t*) value;
	const group_message_t* message = (const group_message_t*) cls;
	
	if (connection != message->sender) {
		struct GNUNET_MessageHeader *header;
		struct GNUNET_MQ_Envelope* env = NULL;
		
		env = GNUNET_MQ_msg_extra(header, message->content_length, GNUNET_MESSAGE_TYPE_CADET_CLI);
		
		GNUNET_memcpy(&(header[1]), message->content, message->content_length);
		
		struct GNUNET_MQ_Handle* mq = GNUNET_CADET_get_mq(connection->channel);
		
		GNUNET_MQ_send(mq, env);
		GNUNET_MQ_notify_sent(env, NULL, NULL);
	}
	
	return GNUNET_YES;
}

static void CGTK_group_send_message(const connection_t* sender, const struct GNUNET_HashCode* group, const msg_t* msg) {
	group_message_t message;
	
	message.sender = sender;
	message.content = CGTK_encode_message(msg, &(message.content_length));
	
	if (message.content_length > 0) {
		GNUNET_CONTAINER_multihashmap_get_multiple(session.groups, group, CGTK_group_push_message, (void*) &message);
	}
	
	free((void*) message.content);
}

static void CGTK_group_recv_message(const connection_t* receiver, const msg_t* msg) {
	group_message_t message;
	
	message.sender = NULL;
	message.content = CGTK_encode_message(msg, &(message.content_length));
	
	if (message.content_length > 0) {
		CGTK_group_push_message((void*) &message, receiver->group, (void*) receiver);
	}
	
	free((void*) message.content);
}

static int CGTK_group_count(void* cls, const struct GNUNET_HashCode* group, void* value) {
	return GNUNET_YES;
}

static int CGTK_group_participants(void* cls, const struct GNUNET_HashCode* group, void* value) {
	connection_t* connection = (connection_t*) value;
	char*** participants = (char***) cls;
	
	**participants = connection->name;
	*participants = (*participants + 1);
	
	return GNUNET_YES;
}

static void* CGTK_on_connect(void* cls, struct GNUNET_CADET_Channel* channel, const struct GNUNET_PeerIdentity* source) {
	const struct GNUNET_HashCode* group = (const struct GNUNET_HashCode*) cls;
	
	connection_t* connection = CGTK_connection_create(source, (group? group : &(session.port)), channel);
	
	int res;
	
	if (group) {
		if (session.groups) {
			msg_t join_msg = {};
			
			join_msg.kind = MSG_KIND_JOIN;
			join_msg.timestamp = time(NULL);
			join_msg.who = connection->name;
			
			CGTK_group_send_message(connection, group, &join_msg);
		}
		
		res = CGTK_group_add_new_connection(connection);
	} else {
		res = CGTK_add_new_connection(connection);
	}
	
	if (res == GNUNET_SYSERR) {
		CGTK_connection_destroy(connection, true);
		return NULL;
	}
	
	if (connection->group) {
		size_t group_size = GNUNET_CONTAINER_multihashmap_get_multiple(session.groups, group, CGTK_group_count, &group_size);
		
		msg_t info_msg = {};
		
		info_msg.kind = MSG_KIND_INFO;
		info_msg.timestamp = time(NULL);
		info_msg.participants = GNUNET_malloc((group_size + 1) * sizeof(char*));
		info_msg.participants[group_size] = NULL;
		
		GNUNET_CONTAINER_multihashmap_get_multiple(session.groups, group, CGTK_group_participants, &(info_msg.participants));
		
		CGTK_group_recv_message(connection, &info_msg);
	} else {
		CGTK_send_gtk_connect(messaging, &(connection->identity), &(connection->port));
	}
	
	return connection;
}

static void CGTK_on_disconnect(void* cls, const struct GNUNET_CADET_Channel* channel) {
	connection_t* connection = (connection_t*) cls;
	
	if (connection->group) {
		CGTK_group_remove_connection(connection);
		
		if (session.groups) {
			msg_t leave_msg = {};
			
			leave_msg.kind = MSG_KIND_LEAVE;
			leave_msg.timestamp = time(NULL);
			leave_msg.who = connection->name;
			
			CGTK_group_send_message(connection, connection->group, &leave_msg);
		}
	} else {
		CGTK_send_gtk_disconnect(messaging, &(connection->identity), &(connection->port));
		CGTK_remove_connection(connection);
	}
	
	CGTK_connection_destroy(connection, false);
}

static void CGTK_handle_message(connection_t* connection, const struct GNUNET_MessageHeader* message) {
	uint16_t length = ntohs(message->size) - sizeof(*message);
	
	const char* buffer = (const char *) &message[1];
	
	GNUNET_CADET_receive_done(connection->channel);
	
	if (connection->group) {
		msg_t* msg = CGTK_decode_message(buffer, length);
		
		CGTK_repair_message(msg, buffer);
		
		if (msg->kind == MSG_KIND_TALK) {
			const char* origin_sender = msg->sender;
			
			msg->sender = connection->name;
			
			CGTK_group_send_message(connection, connection->group, msg);
			
			msg->sender = origin_sender;
		}
		
		CGTK_free_message(msg);
	} else {
		ssize_t result = CGTK_send_gtk_message(messaging, &(connection->identity), &(connection->port), buffer, length);
		
		if (result < 0) {
			CGTK_fatal_error("No connection!\0");
		}
	}
}

static int check_channel_message(void* cls, const struct GNUNET_MessageHeader* message) {
	connection_t* connection = (connection_t*) cls;
	
	if (connection->group) {
		return (session.groups) && (GNUNET_CONTAINER_multihashmap_contains_value(
				session.groups, connection->group, connection
		))? GNUNET_OK : GNUNET_NO;
	} else {
		return (session.connections) && (GNUNET_CONTAINER_multipeermap_contains_value(
				session.connections, &(connection->identity), connection
		))? GNUNET_OK : GNUNET_NO;
	}
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

static group_t* CGTK_group_create(const struct GNUNET_HashCode* port) {
	struct GNUNET_MQ_MessageHandler handlers[] = {
			GNUNET_MQ_hd_var_size(
					channel_message,
					GNUNET_MESSAGE_TYPE_CADET_CLI,
					struct GNUNET_MessageHeader,
					NULL
			), GNUNET_MQ_handler_end()
	};
	
	group_t* group = GNUNET_malloc(sizeof(group_t));
	
	GNUNET_memcpy(&(group->port), port, sizeof(struct GNUNET_HashCode));
	
	group->listen = GNUNET_CADET_open_port(
			session.cadet,
			&(group->port),
			&CGTK_on_connect,
			(void*) &(group->port),
			NULL,
			&CGTK_on_disconnect,
			handlers
	);
	
	return group;
}

static void CGTK_group_destroy(group_t* group) {
	GNUNET_CADET_close_port(group->listen);
	
	GNUNET_free(group);
}

static bool CGTK_group_open(const struct GNUNET_HashCode* port) {
	if (session.group_ports) {
		if (GNUNET_CONTAINER_multihashmap_contains(session.group_ports, port) == GNUNET_YES) {
			return false;
		}
	} else {
		session.group_ports = GNUNET_CONTAINER_multihashmap_create(8, GNUNET_NO);
	}
	
	group_t* group = CGTK_group_create(port);
	
	int res = GNUNET_CONTAINER_multihashmap_put(
			session.group_ports, &(group->port), group, GNUNET_CONTAINER_MULTIHASHMAPOPTION_UNIQUE_FAST
	);
	
	if (res == GNUNET_SYSERR) {
		CGTK_group_destroy(group);
	}
	
	return (res == GNUNET_YES);
}

static bool CGTK_group_close(const struct GNUNET_HashCode* port) {
	if (session.groups) {
		GNUNET_CONTAINER_multihashmap_get_multiple(session.groups, port, CGTK_group_clear_connection, NULL);
		GNUNET_CONTAINER_multihashmap_remove_all(session.groups, port);
	}
	
	if (session.group_ports) {
		group_t* group = (group_t*) GNUNET_CONTAINER_multihashmap_get(
				session.group_ports, port
		);
		
		if (group) {
			CGTK_group_destroy(group);
		}
		
		return (GNUNET_CONTAINER_multihashmap_remove_all(session.group_ports, port) == GNUNET_YES);
	} else {
		return false;
	}
}

static void CGTK_done_message(void* cls) {
	if (session.idle) {
		GNUNET_SCHEDULER_cancel(session.idle);
	}
	
	CGTK_idle(cls);
}

static bool CGTK_push_message(connection_t* connection) {
	size_t length = CGTK_recv_gtk_msg_length(messaging);
	char buffer[CGTK_MESSAGE_BUFFER_SIZE + 1];
	
	struct GNUNET_MessageHeader *msg;
	struct GNUNET_MQ_Envelope* env = NULL;
	
	env = GNUNET_MQ_msg_extra(msg, length, GNUNET_MESSAGE_TYPE_CADET_CLI);
	
	size_t complete = 0;
	
	while (complete < length) {
		ssize_t offset = 0;
		size_t remaining = length - complete;
		
		if (remaining > CGTK_MESSAGE_BUFFER_SIZE) {
			remaining = CGTK_MESSAGE_BUFFER_SIZE;
		}
		
		while (offset < remaining) {
			ssize_t done = CGTK_recv_gtk_message(messaging, buffer + offset, remaining - offset);
			
			if (done <= 0) {
				CGTK_fatal_error("Connection lost!\0");
				return false;
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
	GNUNET_MQ_notify_sent(env, NULL, NULL);
	
	return true;
}

static int CGTK_send_message(void *cls, const struct GNUNET_PeerIdentity* identity, void* value) {
	connection_t* connection = (connection_t*) value;
	
	if (GNUNET_CRYPTO_hash_cmp(&(connection->port), (struct GNUNET_HashCode*) cls) == 0) {
		CGTK_push_message(connection);
		return GNUNET_NO;
	} else {
		return GNUNET_YES;
	}
}

static int CGTK_exit_connection(void *cls, const struct GNUNET_PeerIdentity* identity, void* value) {
	connection_t* connection = (connection_t*) value;
	
	if (GNUNET_CRYPTO_hash_cmp(&(connection->port), (struct GNUNET_HashCode*) cls) == 0) {
		CGTK_remove_connection(connection);
		
		CGTK_connection_destroy(connection, true);
		return GNUNET_NO;
	} else {
		return GNUNET_YES;
	}
}

static void CGTK_idle(void* cls) {
	session.idle = NULL;
	
	msg_type_t type = CGTK_recv_gtk_msg_type(messaging);
	
	switch (type) {
		case MSG_GNUNET_PORT: {
			const struct GNUNET_HashCode* port = CGTK_recv_gtk_hashcode(messaging);
			
			if (!port) {
				CGTK_fatal_error("Can't identify connections port!\0");
				return;
			}
			
			if (session.listen) {
				GNUNET_CADET_close_port(session.listen);
				session.listen = NULL;
			}
			
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
					NULL,
					&CGTK_on_disconnect,
					handlers
			);
			
			break;
		} case MSG_GNUNET_GROUP: {
			const struct GNUNET_HashCode* port = CGTK_recv_gtk_hashcode(messaging);
			
			if (!port) {
				CGTK_fatal_error("Can't identify groups port!\0");
				return;
			}
			
			if (CGTK_group_open(port)) {
				struct GNUNET_PeerIdentity peer;
				
				if (GNUNET_CRYPTO_get_peer_identity(session.cfg, &peer) != GNUNET_OK) {
					memset(&peer, 0, sizeof(peer));
				}
				
				CGTK_send_gtk_connect(messaging, &peer, port);
			}
			
			break;
		} case MSG_GNUNET_EXIT: {
			const struct GNUNET_PeerIdentity* identity = CGTK_recv_gtk_identity(messaging);
			
			if (!identity) {
				CGTK_fatal_error("Can't identify connections identity!\0");
				return;
			}
			
			const struct GNUNET_HashCode *port = CGTK_recv_gtk_hashcode(messaging);
			
			if (!port) {
				CGTK_fatal_error("Can't identify connections port!\0");
				return;
			}
			
			if (session.connections) {
				struct GNUNET_HashCode hashcode;
				GNUNET_memcpy(&hashcode, port, sizeof(struct GNUNET_HashCode));
				
				GNUNET_CONTAINER_multipeermap_get_multiple(session.connections, identity, CGTK_exit_connection, &hashcode);
			}
			
			if (session.group_ports) {
				const char* dest_s = GNUNET_i2s_full(identity);
				char dest_string [GNUNET_CRYPTO_PKEY_ASCII_LENGTH + 1];
				
				strncpy(dest_string, dest_s, GNUNET_CRYPTO_PKEY_ASCII_LENGTH);
				dest_string[GNUNET_CRYPTO_PKEY_ASCII_LENGTH] = '\0';
				
				struct GNUNET_PeerIdentity peer;
				
				if (GNUNET_CRYPTO_get_peer_identity(session.cfg, &peer) != GNUNET_OK) {
					memset(&peer, 0, sizeof(peer));
				}
				
				const char* peer_string = GNUNET_i2s_full(&peer);
				
				if (strcmp(dest_string, peer_string) == 0) {
					CGTK_group_close(port);
				}
			}
			
			break;
		} case MSG_GNUNET_SEND_MESSAGE: {
			const struct GNUNET_PeerIdentity *destination = CGTK_recv_gtk_identity(messaging);
			
			if (!destination) {
				CGTK_fatal_error("Can't identify connections destination!\0");
				return;
			}
			
			const struct GNUNET_HashCode *port = CGTK_recv_gtk_hashcode(messaging);
			
			if (!port) {
				CGTK_fatal_error("Can't identify connections port!\0");
				return;
			}
			
			struct GNUNET_HashCode hashcode;
			GNUNET_memcpy(&hashcode, port, sizeof(struct GNUNET_HashCode));
			
			int search_destination = (session.connections? GNUNET_CONTAINER_multipeermap_get_multiple(
					session.connections,
					destination,
					CGTK_send_message,
					&hashcode
			) : 0);
			
			if (search_destination != GNUNET_SYSERR) {
				struct GNUNET_MQ_MessageHandler handlers[] = {
						GNUNET_MQ_hd_var_size(
								channel_message,
								GNUNET_MESSAGE_TYPE_CADET_CLI,
								struct GNUNET_MessageHeader,
								NULL
						), GNUNET_MQ_handler_end()
				};
				
				connection_t* connection = CGTK_connection_create(destination, port, NULL);
				
				connection->channel = GNUNET_CADET_channel_create(
						session.cadet,
						connection,
						&(connection->identity),
						&(connection->port),
						NULL,
						&CGTK_on_disconnect,
						handlers
				);
				
				if ((!CGTK_push_message(connection)) || (CGTK_add_new_connection(connection) == GNUNET_SYSERR)) {
					CGTK_connection_destroy(connection, true);
					connection = NULL;
					break;
				}
			}
			
			break;
		} case MSG_ERROR: {
			CGTK_fatal_error(NULL);
			return;
		} default: {
			break;
		}
	}
	
	session.idle = GNUNET_SCHEDULER_add_delayed_with_priority(
			session.delay,
			GNUNET_SCHEDULER_PRIORITY_IDLE,
			&CGTK_idle,
			NULL
	);
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
		CGTK_fatal_error("Service unavailable!\0");
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
			NULL,
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
