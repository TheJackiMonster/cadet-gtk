//
// Created by thejackimonster on 12.06.20.
//

struct request_t {
	struct request_t* prev;
	struct request_t* next;
	
	struct GNUNET_FS_Uri* uri;
	char path [PATH_MAX];
	
	float progress;
	
	connection_t* connection;
	
	struct GNUNET_FS_DownloadContext* context;
};

static request_t* CGTK_request_create(connection_t* connection, struct GNUNET_FS_Uri* uri) {
	request_t* request = GNUNET_malloc(sizeof(request_t));
	memset(request, 0, sizeof(publication_t));
	
	request->uri = uri;
	
	const char* path = CGTK_generate_random_filename();
	
	path = CGTK_storage_file_path(CGTK_STORAGE_DOWNLOAD_DIR, path);
	GNUNET_strlcpy(request->path, path, PATH_MAX);
	
	request->progress = 0.0f;
	
	request->connection = connection;
	
	return request;
}

static void CGTK_request_destroy(request_t* request) {
	if (request->context) {
		GNUNET_FS_download_stop(request->context, FALSE);
		request->context = NULL;
	}
	
	if (request->uri) {
		GNUNET_FS_uri_destroy(request->uri);
		request->uri = NULL;
	}
	
	GNUNET_free(request);
}

static void CGTK_request_progress(void* cls) {
	request_t* request = (request_t*) cls;
	
	const struct GNUNET_PeerIdentity* destination = GNUNET_PEER_resolve2(request->connection->identity);
	
	CGTK_send_gui_file_progress(messaging, destination, &(request->connection->port), request->progress, NULL, request->uri);
}

static void CGTK_request_finish(void* cls) {
	request_t* request = (request_t*) cls;
	
	const struct GNUNET_PeerIdentity* destination = GNUNET_PEER_resolve2(request->connection->identity);
	
	CGTK_send_gui_file_complete(messaging, destination, &(request->connection->port), request->path, request->uri);
	
	GNUNET_CONTAINER_DLL_remove(session.requests_head, session.requests_tail, request);
	
	CGTK_request_destroy(request);
}
