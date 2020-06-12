//
// Created by thejackimonster on 07.06.20.
//

#include "../storage.h"

struct publication_t {
	struct publication_t* prev;
	struct publication_t* next;
	
	char path [PATH_MAX];
	struct GNUNET_FS_Uri* uri;
	
	cgtk_1tu_key_t key;
	float progress;
	
	connection_t* connection;
	
	struct GNUNET_CONTAINER_MetaData* meta;
	struct GNUNET_FS_PublishContext* context;
};

static publication_t* CGTK_publication_create(connection_t* connection, const char* path) {
	publication_t* publication = GNUNET_malloc(sizeof(publication_t));
	memset(publication, 0, sizeof(publication_t));
	
	GNUNET_strlcpy(publication->path, path, PATH_MAX);
	
	CGTK_generate_new_key(&(publication->key));
	publication->progress = 0.0f;
	
	publication->connection = connection;
	publication->meta = GNUNET_CONTAINER_meta_data_create();
	
	return publication;
}

static void CGTK_publication_destroy(publication_t* publication) {
	CGTK_wipe_key(&(publication->key));
	
	if (publication->context) {
		GNUNET_FS_publish_stop(publication->context);
		publication->context = NULL;
	}
	
	if (publication->meta) {
		GNUNET_CONTAINER_meta_data_destroy(publication->meta);
		publication->meta = NULL;
	}
	
	if (publication->uri) {
		GNUNET_FS_uri_destroy(publication->uri);
		publication->uri = NULL;
	}
	
	GNUNET_free(publication);
}

static void CGTK_publication_progress(void* cls) {
	publication_t* publication = (publication_t*) cls;
	
	const struct GNUNET_PeerIdentity* destination = GNUNET_PEER_resolve2(publication->connection->identity);
	
	CGTK_send_gui_file_progress(messaging, destination, &(publication->connection->port), publication->progress, publication->path, NULL);
}

static void CGTK_publication_finish(void* cls) {
	publication_t* publication = (publication_t*) cls;
	
	const struct GNUNET_PeerIdentity* destination = GNUNET_PEER_resolve2(publication->connection->identity);
	
	CGTK_send_gui_file_complete(messaging, destination, &(publication->connection->port), publication->path, publication->uri);
	
	GNUNET_CONTAINER_DLL_remove(session.publications_head, session.publications_tail, publication);
	
	CGTK_publication_destroy(publication);
}
