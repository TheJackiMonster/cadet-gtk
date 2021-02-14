//
// Created by thejackimonster on 14.04.20.
//

#include "gnunet.h"

#include "config.h"
#include "msg.h"
#include "messaging.h"

static messaging_t* messaging;

typedef struct publication_t publication_t;
typedef struct request_t request_t;

static struct {
	const struct GNUNET_CONFIGURATION_Handle* cfg;
	
	struct {
		struct GNUNET_ARM_Handle* arm;
		struct GNUNET_CADET_Handle* cadet;
		struct GNUNET_FS_Handle* fs;
	} handles;
	
	struct GNUNET_REGEX_Announcement* name_announcement;
	struct GNUNET_REGEX_Search* name_search;
	char* name_needle;
	
	struct GNUNET_HashCode port;
	struct GNUNET_CADET_Port* listen;
	
	struct GNUNET_CONTAINER_MultiPeerMap* connections;
	struct GNUNET_CONTAINER_MultiHashMap* group_ports;
	struct GNUNET_CONTAINER_MultiHashMap* groups;
	
	publication_t* publications_head;
	publication_t* publications_tail;
	
	request_t* requests_head;
	request_t* requests_tail;
	
	struct GNUNET_SCHEDULER_Task* idle;
	struct GNUNET_TIME_Relative delay;
} session;

static void CGTK_fatal_error(const char* error_message) {
	GNUNET_SCHEDULER_shutdown();
	
	if (error_message) {
		perror(error_message);
	}
}

#include "gnunet/name.c"
#include "gnunet/connection.c"
#include "gnunet/group.c"
#include "gnunet/publication.c"
#include "gnunet/request.c"

static void CGTK_shutdown(void* cls) {
#ifdef CGTK_ALL_DEBUG
	printf("GNUNET: CGTK_shutdown()\n");
#endif
	
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
	
	do {
		publication_t* publication = session.publications_head;
		
		if (publication) {
			GNUNET_CONTAINER_DLL_remove(session.publications_head, session.publications_tail, publication);
			
			CGTK_publication_destroy(publication);
		}
	} while (session.publications_head);
	
	do {
		request_t* request = session.requests_head;
		
		if (request) {
			GNUNET_CONTAINER_DLL_remove(session.requests_head, session.requests_tail, request);
			
			CGTK_request_destroy(request);
		}
	} while (session.requests_head);
	
	if (session.listen) {
		GNUNET_CADET_close_port(session.listen);
		session.listen = NULL;
	}
	
	if (session.handles.fs) {
		GNUNET_FS_stop(session.handles.fs);
		session.handles.fs = NULL;
	}
	
	if (session.handles.cadet) {
		GNUNET_CADET_disconnect(session.handles.cadet);
		session.handles.cadet = NULL;
	}
	
	if (session.handles.arm) {
		GNUNET_ARM_disconnect(session.handles.arm);
		session.handles.arm = NULL;
	}
	
	CGTK_close_messaging(messaging);
}

static void CGTK_idle(void* cls);

static void* CGTK_on_connect(void* cls, struct GNUNET_CADET_Channel* channel, const struct GNUNET_PeerIdentity* source) {
#ifdef CGTK_ALL_DEBUG
	printf("GNUNET: CGTK_on_connect()\n");
#endif
	
	const struct GNUNET_HashCode* group = (const struct GNUNET_HashCode*) cls;
	
	connection_t* connection = CGTK_connection_create(source, (group? group : &(session.port)), channel);
	
	int res;
	
	if (group) {
		if (session.groups) {
			msg_t join_msg = {};
			
			join_msg.kind = MSG_KIND_JOIN;
			join_msg.timestamp = time(NULL);
			join_msg.join_leave.who = connection->name;
			
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
		info_msg.info.participants = GNUNET_malloc((group_size + 1) * sizeof(char*));
		info_msg.info.participants[group_size] = NULL;
		
		GNUNET_CONTAINER_multihashmap_get_multiple(session.groups, group, CGTK_group_participants, &(info_msg.info.participants));
		
		CGTK_group_recv_message(connection, &info_msg);
	} else {
		CGTK_send_gui_connect(messaging, GNUNET_PEER_resolve2(connection->identity), &(connection->port));
	}
	
	return connection;
}

static void CGTK_on_disconnect(void* cls, const struct GNUNET_CADET_Channel* channel) {
#ifdef CGTK_ALL_DEBUG
	printf("GNUNET: CGTK_on_disconnect()\n");
#endif
	
	connection_t* connection = (connection_t*) cls;
	
	if (connection->group) {
		CGTK_group_remove_connection(connection);
		
		if (session.groups) {
			msg_t leave_msg = {};
			
			leave_msg.kind = MSG_KIND_LEAVE;
			leave_msg.timestamp = time(NULL);
			leave_msg.join_leave.who = connection->name;
			
			CGTK_group_send_message(connection, connection->group, &leave_msg);
		}
	} else {
		CGTK_send_gui_disconnect(messaging, GNUNET_PEER_resolve2(connection->identity), &(connection->port));
		CGTK_remove_connection(connection);
	}
	
	CGTK_connection_destroy(connection, false);
}

static void CGTK_handle_message(connection_t* connection, const struct GNUNET_MessageHeader* message) {
#ifdef CGTK_ALL_DEBUG
	printf("GNUNET: CGTK_handle_message()\n");
#endif
	
	uint16_t length = ntohs(message->size) - sizeof(*message);
	
	const char* buffer = (const char *) &message[1];
	
	GNUNET_CADET_receive_done(connection->channel);
	
	if (connection->group) {
		msg_t* msg = CGTK_decode_message(buffer, length);
		
		CGTK_repair_message(msg, buffer, length, connection->name);
		
		if (msg->kind == MSG_KIND_TALK) {
			const char* origin_sender = msg->talk.sender;
			
			msg->talk.sender = connection->name;
			
			CGTK_group_send_message(connection, connection->group, msg);
			
			msg->talk.sender = origin_sender;
		}
		
		CGTK_free_message(msg);
	} else {
		ssize_t result = CGTK_send_gui_message(messaging, GNUNET_PEER_resolve2(connection->identity), &(connection->port), buffer, length);
		
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

static bool CGTK_push_message(connection_t* connection) {
#ifdef CGTK_ALL_DEBUG
	printf("GNUNET: CGTK_push_message()\n");
#endif
	
	size_t length = CGTK_recv_gui_msg_length(messaging);
	char buffer [CGTK_MESSAGE_BUFFER_SIZE];
	
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
			ssize_t done = CGTK_recv_gui_message(messaging, buffer + offset, remaining - offset);
			
			if (done <= 0) {
				CGTK_fatal_error("Connection lost!\0");
				return false;
			}
			
			offset += done;
		}
		
		if (env) {
			GNUNET_memcpy(&(msg[1]) + complete, buffer, offset);
		}
		
		complete += offset;
	}
	
	struct GNUNET_MQ_Handle* mq = GNUNET_CADET_get_mq(connection->channel);
	
	GNUNET_MQ_send(mq, env);
	GNUNET_MQ_notify_sent(env, NULL, NULL);
	
	return true;
}

static void CGTK_upload_publication(const char* path) {
#ifdef CGTK_ALL_DEBUG
	printf("GNUNET: CGTK_push_upload()\n");
#endif
	
	publication_t* publication = CGTK_publication_create(path);
	
	struct GNUNET_FS_BlockOptions bo = {};
	
	bo.expiration_time = GNUNET_TIME_absolute_add(GNUNET_TIME_absolute_get(), GNUNET_TIME_UNIT_HOURS); // 1 hour expiration as default?
	bo.anonymity_level = 1; // default anonymity?
	bo.content_priority = 100; // default priority?
	bo.replication_level = 1; // we want to send the file to one peer probably?
	
	struct GNUNET_FS_FileInformation* fi = GNUNET_FS_file_information_create_from_file(
			session.handles.fs,
			publication,
			path,
			NULL,
			publication->meta,
			GNUNET_YES,
			&bo
	);
	
	publication->context.publish = GNUNET_FS_publish_start(
			session.handles.fs, fi,
			NULL, NULL, NULL,
			GNUNET_FS_PUBLISH_OPTION_NONE
	);
	
	GNUNET_CONTAINER_DLL_insert(session.publications_head, session.publications_tail, publication);
}

static void CGTK_unindex_publication(const char* path) {
#ifdef CGTK_ALL_DEBUG
	printf("GNUNET: CGTK_unindex_publication()\n");
#endif
	
	publication_t* publication = CGTK_publication_create(path);
	
	publication->context.unindex = GNUNET_FS_unindex_start(session.handles.fs, path, publication);
	
	GNUNET_CONTAINER_DLL_insert(session.publications_head, session.publications_tail, publication);
}

static void CGTK_request_download(struct GNUNET_FS_Uri* uri, const char* path) {
#ifdef CGTK_ALL_DEBUG
	printf("GNUNET: CGTK_request_download()\n");
#endif
	
	request_t* request = CGTK_request_create(uri, path);
	
	uint64_t offset = 0LU;
	
	GNUNET_DISK_file_size(path, &offset, FALSE, TRUE);
	
	const uint64_t amount = GNUNET_FS_uri_chk_get_file_size(request->uri) - offset;
	
	request->context = GNUNET_FS_download_start(
			session.handles.fs,
			request->uri,
			NULL,
			request->path,
			NULL,
			offset, // continue from suspended download?
			amount, // rest of the complete size?
			1, // default anonymity?
			GNUNET_FS_DOWNLOAD_OPTION_NONE,
			request,
			NULL
	);
	
	GNUNET_CONTAINER_DLL_insert(session.requests_head, session.requests_tail, request);
}

typedef struct {
	struct GNUNET_HashCode hashcode;
	bool (*call)(connection_t* connection);
} call_by_connection_t;

static int CGTK_call_by_connection_search(void* cls, const struct GNUNET_PeerIdentity* identity, void* value) {
#ifdef CGTK_ALL_DEBUG
	printf("GNUNET: CGTK_call_by_connection_search()\n");
#endif
	
	connection_t* connection = (connection_t*) value;
	call_by_connection_t* call_info = (call_by_connection_t*) cls;
	
	if (GNUNET_CRYPTO_hash_cmp(&(connection->port), &(call_info->hashcode)) == 0) {
		call_info->call(connection);
		return GNUNET_NO;
	} else {
		return GNUNET_YES;
	}
}

static bool CGTK_call_by_connection(const struct GNUNET_PeerIdentity* identity, const struct GNUNET_HashCode* port, bool (*call)(connection_t* connection)) {
#ifdef CGTK_ALL_DEBUG
	printf("GNUNET: CGTK_call_by_connection()\n");
#endif
	
	call_by_connection_t call_info;
	
	GNUNET_memcpy(&(call_info.hashcode), port, sizeof(struct GNUNET_HashCode));
	call_info.call = call;
	
	int search_connection = (session.connections ? GNUNET_CONTAINER_multipeermap_get_multiple(
			session.connections,
			identity,
			CGTK_call_by_connection_search,
			&call_info
	) : 0);
	
	if (search_connection != GNUNET_SYSERR) {
		struct GNUNET_MQ_MessageHandler handlers[] = {
				GNUNET_MQ_hd_var_size(
						channel_message,
						GNUNET_MESSAGE_TYPE_CADET_CLI,
						struct GNUNET_MessageHeader,
						NULL
				), GNUNET_MQ_handler_end()
		};
		
		connection_t *connection = CGTK_connection_create(identity, port, NULL);
		
		connection->channel = GNUNET_CADET_channel_create(
				session.handles.cadet,
				connection,
				GNUNET_PEER_resolve2(connection->identity),
				&(connection->port),
				NULL,
				&CGTK_on_disconnect,
				handlers
		);
		
		if ((!call(connection)) || (CGTK_add_new_connection(connection) == GNUNET_SYSERR)) {
			CGTK_connection_destroy(connection, true);
			connection = NULL;
			return false;
		}
	}
	
	return true;
}

static const char* CGTK_receive_name() {
#ifdef CGTK_ALL_DEBUG
	printf("GNUNET: CGTK_receive_name()\n");
#endif
	
	static char buffer[CGTK_NAME_BUFFER_SIZE];
	
	size_t length = CGTK_recv_gui_msg_length(messaging);
	
	if (length >= CGTK_NAME_BUFFER_SIZE) {
		length = CGTK_NAME_BUFFER_SIZE - 1;
	}
	
	ssize_t offset = 0;
	
	while (offset < length) {
		ssize_t done = CGTK_recv_gui_message(messaging, buffer + offset, length - offset);
		
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
	
	msg_type_t type = CGTK_recv_gui_msg_type(messaging);
	
	switch (type) {
		case MSG_GNUNET_HOST: {
#ifdef CGTK_ALL_DEBUG
			printf("GNUNET: CGTK_idle(): MSG_GNUNET_HOST\n");
#endif
			
			const uint8_t visibility = CGTK_recv_gui_code(messaging);
			
			const struct GNUNET_HashCode *port = CGTK_recv_gui_hashcode(messaging);
			
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
			
			if (visibility != CGTK_VISIBILITY_CAT) {
				struct GNUNET_MQ_MessageHandler handlers[] = {
						GNUNET_MQ_hd_var_size(
								port_message,
								GNUNET_MESSAGE_TYPE_CADET_CLI,
								struct GNUNET_MessageHeader,
								NULL
						), GNUNET_MQ_handler_end()
				};
				
				session.listen = GNUNET_CADET_open_port(
						session.handles.cadet,
						&(session.port),
						&CGTK_on_connect,
						NULL,
						NULL,
						&CGTK_on_disconnect,
						handlers
				);
			}
			
			if (visibility != CGTK_VISIBILITY_PUBLIC) {
				name_regex = "\0";
			}
			
			CGTK_name_call(name_regex);
			break;
		} case MSG_GNUNET_SEARCH: {
#ifdef CGTK_ALL_DEBUG
			printf("GNUNET: CGTK_idle(): MSG_GNUNET_SEARCH\n");
#endif
			
			const char* name = CGTK_receive_name();
			
			CGTK_name_search(name);
			break;
		} case MSG_GNUNET_GROUP: {
#ifdef CGTK_ALL_DEBUG
			printf("GNUNET: CGTK_idle(): MSG_GNUNET_GROUP\n");
#endif
			
			const struct GNUNET_HashCode* port = CGTK_recv_gui_hashcode(messaging);
			
			if (!port) {
				CGTK_fatal_error("Can't identify groups port!\0");
				return;
			}
			
			if (CGTK_group_open(port)) {
				struct GNUNET_PeerIdentity peer;
				
				if (GNUNET_CRYPTO_get_peer_identity(session.cfg, &peer) != GNUNET_OK) {
					memset(&peer, 0, sizeof(peer));
				}
				
				CGTK_send_gui_connect(messaging, &peer, port);
			}
			
			break;
		} case MSG_GNUNET_EXIT: {
#ifdef CGTK_ALL_DEBUG
			printf("GNUNET: CGTK_idle(): MSG_GNUNET_EXIT\n");
#endif
			
			const struct GNUNET_PeerIdentity* identity = CGTK_recv_gui_identity(messaging);
			
			if (!identity) {
				CGTK_fatal_error("Can't identify connections identity!\0");
				return;
			}
			
			const struct GNUNET_HashCode *port = CGTK_recv_gui_hashcode(messaging);
			
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
#ifdef CGTK_ALL_DEBUG
			printf("GNUNET: CGTK_idle(): MSG_GNUNET_SEND_MESSAGE\n");
#endif
			
			const struct GNUNET_PeerIdentity* destination = CGTK_recv_gui_identity(messaging);
			
			if (!destination) {
				CGTK_fatal_error("Can't identify connections destination!\0");
				return;
			}
			
			const struct GNUNET_HashCode* port = CGTK_recv_gui_hashcode(messaging);
			
			if (!port) {
				CGTK_fatal_error("Can't identify connections port!\0");
				return;
			}
			
			CGTK_call_by_connection(destination, port, CGTK_push_message);
			break;
		} case MSG_GNUNET_UPLOAD_FILE: {
#ifdef CGTK_ALL_DEBUG
			printf("GNUNET: CGTK_idle(): MSG_GNUNET_UPLOAD_FILE\n");
#endif
			
			const char* path = CGTK_recv_gui_path(messaging);
			
			if (!path) {
				CGTK_fatal_error("Can't identify files path!\0");
				return;
			}
			
			CGTK_upload_publication(path);
			break;
		} case MSG_GNUNET_DOWNLOAD_FILE: {
#ifdef CGTK_ALL_DEBUG
			printf("GNUNET: CGTK_idle(): MSG_GNUNET_DOWNLOAD_FILE\n");
#endif
			
			struct GNUNET_FS_Uri *uri = CGTK_recv_gui_uri(messaging);
			
			if (!uri) {
				CGTK_fatal_error("Can't identify files uri!\0");
				return;
			}
			
			const char *path = CGTK_recv_gui_path(messaging);
			
			if (!path) {
				GNUNET_FS_uri_destroy(uri);
				
				CGTK_fatal_error("Can't identify files path!\0");
				return;
			}
			
			if (!GNUNET_FS_uri_test_chk(uri)) {
				GNUNET_FS_uri_destroy(uri);
				break;
			}
			
			CGTK_request_download(uri, path);
			break;
		} case MSG_GNUNET_UNINDEX_FILE: {
#ifdef CGTK_ALL_DEBUG
			printf("GNUNET: CGTK_idle(): MSG_GNUNET_UNINDEX_FILE\n");
#endif
			
			const char* path = CGTK_recv_gui_path(messaging);
			
			if (!path) {
				CGTK_fatal_error("Can't identify files path!\0");
				return;
			}
			
			CGTK_unindex_publication(path);
			break;
		} case MSG_ERROR: {
#ifdef CGTK_ALL_DEBUG
			printf("GNUNET: CGTK_idle(): MSG_GNUNET_ERROR\n");
#endif
			
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

static void CGTK_arm_connection(void* cls, int connected) {
#ifdef CGTK_ALL_DEBUG
	printf("GNUNET: CGTK_arm_connection(): %d\n", connected);
#endif
	
	if (connected) {
		GNUNET_ARM_request_service_start(session.handles.arm, "cadet", GNUNET_OS_INHERIT_STD_NONE, NULL, NULL);
		GNUNET_ARM_request_service_start(session.handles.arm, "fs", GNUNET_OS_INHERIT_STD_NONE, NULL, NULL);
	} else {
		GNUNET_ARM_request_service_start(session.handles.arm, "arm", GNUNET_OS_INHERIT_STD_NONE, NULL, NULL);
	}
}

static void* CGTK_fs_progress(void* cls, const struct GNUNET_FS_ProgressInfo* info) {
#ifdef CGTK_ALL_DEBUG
	printf("GNUNET: CGTK_fs_progress(%d)\n", info->status);
#endif
	
	switch (info->status) {
		case GNUNET_FS_STATUS_PUBLISH_START: {
			publication_t* publication = (publication_t*) info->value.publish.cctx;
			publication->progress = 0.0f;
			
			GNUNET_SCHEDULER_add_now(&CGTK_publication_progress, publication);
			
			return publication;
		} case GNUNET_FS_STATUS_PUBLISH_PROGRESS: {
			publication_t* publication = (publication_t*) info->value.publish.cctx;
			publication->progress = 1.0f * info->value.publish.completed / info->value.publish.size;
			
			GNUNET_SCHEDULER_add_now(&CGTK_publication_progress, publication);
			
			return publication;
		} case GNUNET_FS_STATUS_PUBLISH_COMPLETED: {
			publication_t* publication = (publication_t*) info->value.publish.cctx;
			publication->uri = GNUNET_FS_uri_dup(info->value.publish.specifics.completed.chk_uri);
			publication->progress = 1.0f;
			
			GNUNET_SCHEDULER_add_now(&CGTK_publication_finish, publication);
			break;
		} case GNUNET_FS_STATUS_PUBLISH_ERROR: {
			publication_t* publication = (publication_t*) info->value.publish.cctx;
			
			GNUNET_SCHEDULER_add_now(&CGTK_publication_error, publication);
		} case GNUNET_FS_STATUS_DOWNLOAD_START: {
			request_t* request = (request_t*) info->value.download.cctx;
			request->progress = 0.0f;
			
			return request;
		} case GNUNET_FS_STATUS_DOWNLOAD_ACTIVE: {
			return info->value.download.cctx;
		} case GNUNET_FS_STATUS_DOWNLOAD_INACTIVE: {
			return info->value.download.cctx;
		} case GNUNET_FS_STATUS_DOWNLOAD_PROGRESS: {
			request_t* request = (request_t*) info->value.download.cctx;
			request->progress = 1.0f * info->value.download.completed / info->value.download.size;
			
			GNUNET_SCHEDULER_add_now(&CGTK_request_progress, request);
			
			return request;
		} case GNUNET_FS_STATUS_DOWNLOAD_COMPLETED: {
			request_t* request = (request_t*) info->value.download.cctx;
			request->progress = 1.0f;
			
			GNUNET_SCHEDULER_add_now(&CGTK_request_finish, request);
			break;
		} case GNUNET_FS_STATUS_DOWNLOAD_ERROR: {
			request_t *request = (request_t *) info->value.download.cctx;
			
			GNUNET_SCHEDULER_add_now(&CGTK_request_error, request);
			break;
		} case GNUNET_FS_STATUS_UNINDEX_START: {
			publication_t* publication = (publication_t*) info->value.unindex.cctx;
			publication->progress = 0.0f;
			
			return publication;
		} case GNUNET_FS_STATUS_UNINDEX_PROGRESS: {
			publication_t* publication = (publication_t*) info->value.unindex.cctx;
			publication->progress = 1.0f * info->value.unindex.completed / info->value.unindex.size;
			
			return publication;
		} case GNUNET_FS_STATUS_UNINDEX_COMPLETED: {
			publication_t* publication = (publication_t*) info->value.unindex.cctx;
			publication->progress = 1.0f;
			
			GNUNET_SCHEDULER_add_now(&CGTK_publication_unindex_finish, publication);
			break;
		} default: {
			break;
		}
	}
	
	return NULL;
}

void CGTK_run_gnunet(void* cls, char*const* args, const char* cfgfile, const struct GNUNET_CONFIGURATION_Handle* cfg) {
	messaging = (messaging_t*) cls;
	
	session.cfg = cfg;
	
	session.handles.arm = GNUNET_ARM_connect(cfg, &CGTK_arm_connection, NULL);
	
	if (session.handles.arm) {
		CGTK_arm_connection(NULL, FALSE);
	}
	
	session.handles.cadet = GNUNET_CADET_connect(cfg);
	session.handles.fs = GNUNET_FS_start(
			cfg,
			CGTK_APPLICATION_ID,
			&CGTK_fs_progress,
			NULL,
			GNUNET_FS_FLAGS_NONE,
			GNUNET_FS_OPTIONS_END
	);
	
	session.name_announcement = NULL;
	session.name_search = NULL;
	session.name_needle = NULL;
	
	memset(&(session.port), 0, sizeof(struct GNUNET_HashCode));
	session.listen = NULL;
	
	session.connections = NULL;
	session.group_ports = NULL;
	session.groups = NULL;
	
	session.publications_head = NULL;
	session.publications_tail = NULL;
	
	session.idle = NULL;
	
	session.delay = GNUNET_TIME_relative_multiply(
			GNUNET_TIME_UNIT_MILLISECONDS,
			CGTK_GNUNET_SESSION_IDLE_DELAY_MS
	);
	
	if (!session.handles.cadet) {
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
	
	CGTK_send_gui_identity(messaging, &peer);
	
	session.listen = GNUNET_CADET_open_port(
			session.handles.cadet,
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
