//
// Created by thejackimonster on 19.05.20.
//

static connection_t* CGTK_connection_create(const struct GNUNET_PeerIdentity* identity, const struct GNUNET_HashCode* port,
											struct GNUNET_CADET_Channel* channel) {
	connection_t* connection = GNUNET_malloc(sizeof(connection_t));
	
	connection->identity = GNUNET_PEER_intern(identity);
	
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
		GNUNET_PEER_change_rc(connection->identity, -1);
		
		GNUNET_CADET_channel_destroy(connection->channel);
		connection->channel = NULL;
	}
	
	GNUNET_free(connection);
}

static int CGTK_clear_connection(void *cls, const struct GNUNET_PeerIdentity* identity, void* value) {
	CGTK_connection_destroy((connection_t*) value, true);
	return GNUNET_YES;
}

static int CGTK_add_new_connection(connection_t* connection) {
	if (!session.connections) {
		session.connections = GNUNET_CONTAINER_multipeermap_create(8, GNUNET_NO);
	}
	
	return GNUNET_CONTAINER_multipeermap_put(
			session.connections,
			GNUNET_PEER_resolve2(connection->identity),
			connection,
			GNUNET_CONTAINER_MULTIHASHMAPOPTION_MULTIPLE
	);
}

static int CGTK_remove_connection(connection_t* connection) {
	if (session.connections) {
		return GNUNET_CONTAINER_multipeermap_remove(session.connections, GNUNET_PEER_resolve2(connection->identity), connection);
	} else {
		return GNUNET_NO;
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
