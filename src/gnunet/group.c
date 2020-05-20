//
// Created by thejackimonster on 19.05.20.
//

static int check_channel_message(void* cls, const struct GNUNET_MessageHeader* message);
static void handle_channel_message(void* cls, const struct GNUNET_MessageHeader* message);

static void* CGTK_on_connect(void* cls, struct GNUNET_CADET_Channel* channel, const struct GNUNET_PeerIdentity* source);
static void CGTK_on_disconnect(void* cls, const struct GNUNET_CADET_Channel* channel);

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

static int CGTK_group_clear_connection(void* cls, const struct GNUNET_HashCode* group, void* value);

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

static int CGTK_group_clear_connection(void* cls, const struct GNUNET_HashCode* group, void* value) {
	CGTK_connection_destroy((connection_t*) value, true);
	return GNUNET_YES;
}

static int CGTK_group_clear_port(void* cls, const struct GNUNET_HashCode* group, void* value) {
	CGTK_group_destroy((group_t*) value);
	return GNUNET_YES;
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

static int CGTK_group_remove_connection(connection_t* connection) {
	if (session.groups) {
		return GNUNET_CONTAINER_multihashmap_remove(session.groups, connection->group, connection);
	} else {
		return GNUNET_NO;
	}
}

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
