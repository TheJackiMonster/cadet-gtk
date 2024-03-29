//
// Created by thejackimonster on 12.06.20.
//

struct request_t {
	struct request_t* prev;
	struct request_t* next;
	
	struct GNUNET_FS_Uri* uri;
	char path [PATH_MAX];
	
	float progress;
	
	struct GNUNET_FS_DownloadContext* context;
};

static request_t* CGTK_request_create(struct GNUNET_FS_Uri* uri, const char* path) {
#ifdef CGTK_ALL_DEBUG
	printf("GNUNET: CGTK_request_create()\n");
#endif
	
	request_t* request = GNUNET_malloc(sizeof(request_t));
	memset(request, 0, sizeof(request_t));
	
	request->uri = uri;
	
	GNUNET_strlcpy(request->path, path, PATH_MAX);
	
	request->progress = 0.0f;
	
	return request;
}

static void CGTK_request_destroy(request_t* request) {
#ifdef CGTK_ALL_DEBUG
	printf("GNUNET: CGTK_request_destroy()\n");
#endif
	
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
#ifdef CGTK_ALL_DEBUG
	printf("GNUNET: CGTK_request_progress()\n");
#endif
	
	request_t* request = (request_t*) cls;
	
	if ((request) && (*(request->path))) {
		CGTK_send_gui_file_progress(messaging, request->progress, request->path);
	}
}

static void CGTK_request_finish(void* cls) {
#ifdef CGTK_ALL_DEBUG
	printf("GNUNET: CGTK_request_finish()\n");
#endif
	
	request_t* request = (request_t*) cls;
	
	CGTK_send_gui_file_complete(messaging, false, request->path, request->uri);
	
	GNUNET_CONTAINER_DLL_remove(session.requests_head, session.requests_tail, request);
	
	CGTK_request_destroy(request);
}

static void CGTK_request_error(void* cls) {
#ifdef CGTK_ALL_DEBUG
	printf("GNUNET: CGTK_request_error()\n");
#endif
	
	request_t* request = (request_t*) cls;
	
	GNUNET_CONTAINER_DLL_remove(session.requests_head, session.requests_tail, request);
	
	CGTK_request_destroy(request);
}
