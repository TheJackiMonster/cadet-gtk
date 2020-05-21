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
	
	struct GNUNET_REGEX_Announcement* name_announcement;
	struct GNUNET_REGEX_Search* name_search;
	char* name_needle;
	
	struct GNUNET_HashCode port;
	struct GNUNET_CADET_Port* listen;
	
	struct GNUNET_CONTAINER_MultiPeerMap* connections;
	struct GNUNET_CONTAINER_MultiHashMap* group_ports;
	struct GNUNET_CONTAINER_MultiHashMap* groups;
	
	struct GNUNET_SCHEDULER_Task* idle;
	struct GNUNET_TIME_Relative delay;
} session;

typedef struct {
	GNUNET_PEER_Id identity;
	
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

#include "gnunet/name.c"
#include "gnunet/connection.c"
#include "gnunet/group.c"

static void CGTK_shutdown(void* cls) {
	if (session.name_announcement) {
		GNUNET_REGEX_announce_cancel(session.name_announcement);
		session.name_announcement = NULL;
	}
	
	if (session.name_search) {
		GNUNET_REGEX_search_cancel(session.name_search);
		session.name_search = NULL;
	}
	
	if (session.name_needle) {
		GNUNET_free(session.name_needle);
		session.name_needle = NULL;
	}
	
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

static void CGTK_idle(void* cls);

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
		CGTK_send_gtk_connect(messaging, GNUNET_PEER_resolve2(connection->identity), &(connection->port));
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
		CGTK_send_gtk_disconnect(messaging, GNUNET_PEER_resolve2(connection->identity), &(connection->port));
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
		
		CGTK_repair_message(msg, buffer, connection->name);
		
		if (msg->kind == MSG_KIND_TALK) {
			const char* origin_sender = msg->sender;
			
			msg->sender = connection->name;
			
			CGTK_group_send_message(connection, connection->group, msg);
			
			msg->sender = origin_sender;
		}
		
		CGTK_free_message(msg);
	} else {
		ssize_t result = CGTK_send_gtk_message(messaging, GNUNET_PEER_resolve2(connection->identity), &(connection->port), buffer, length);
		
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
				session.connections, GNUNET_PEER_resolve2(connection->identity), connection
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

static const char* CGTK_receive_name() {
	static char buffer[CGTK_NAME_SEARCH_SIZE + 1];
	
	size_t length = CGTK_recv_gtk_msg_length(messaging);
	
	if (length > CGTK_NAME_SEARCH_SIZE) {
		length = CGTK_NAME_SEARCH_SIZE;
	}
	
	ssize_t offset = 0;
	
	while (offset < length) {
		ssize_t done = CGTK_recv_gtk_message(messaging, buffer + offset, length - offset);
		
		if (done <= 0) {
			CGTK_fatal_error("Connection lost!\0");
			return NULL;
		}
		
		offset += done;
	}
	
	buffer[offset] = '\0';
	
	return buffer;
}

static void CGTK_idle(void* cls) {
	session.idle = NULL;
	
	msg_type_t type = CGTK_recv_gtk_msg_type(messaging);
	
	switch (type) {
		case MSG_GNUNET_HOST: {
			const struct GNUNET_HashCode *port = CGTK_recv_gtk_hashcode(messaging);
			
			if (!port) {
				CGTK_fatal_error("Can't identify hosts port!\0");
				return;
			}
			
			const char* name_regex = CGTK_receive_name();
			
			if (!name_regex) {
				CGTK_fatal_error("Can't identify hosts name!\0");
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
			
			CGTK_name_call(name_regex);
			break;
		} case MSG_GNUNET_SEARCH: {
			const char* name = CGTK_receive_name();
			
			CGTK_name_search(name);
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
						GNUNET_PEER_resolve2(connection->identity),
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
	
	session.name_announcement = NULL;
	session.name_search = NULL;
	session.name_needle = NULL;
	
	memset(&(session.port), 0, sizeof(struct GNUNET_HashCode));
	session.listen = NULL;
	
	session.connections = NULL;
	session.group_ports = NULL;
	session.groups = NULL;
	
	session.idle = NULL;
	
	session.delay = GNUNET_TIME_relative_multiply(
			GNUNET_TIME_UNIT_MILLISECONDS,
			CGTK_GNUNET_SESSION_IDLE_DELAY_MS
	);
	
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
	
	session.idle = GNUNET_SCHEDULER_add_delayed_with_priority(
			session.delay,
			GNUNET_SCHEDULER_PRIORITY_IDLE,
			&CGTK_idle,
			NULL
	);
}
