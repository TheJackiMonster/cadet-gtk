//
// Created by thejackimonster on 07.06.20.
//

#include "../storage.h"

struct publication_t {
	struct publication_t* prev;
	struct publication_t* next;
	
	char path [PATH_MAX];
	struct GNUNET_FS_Uri* uri;
	
	float progress;
	
	struct GNUNET_CONTAINER_MetaData* meta;
	struct GNUNET_FS_PublishContext* context;
};

static publication_t* CGTK_publication_create(const char* path) {
	publication_t* publication = GNUNET_malloc(sizeof(publication_t));
	memset(publication, 0, sizeof(publication_t));
	
	GNUNET_strlcpy(publication->path, path, PATH_MAX);
	
	publication->progress = 0.0f;
	
	publication->meta = GNUNET_CONTAINER_meta_data_create();
	
	return publication;
}

static void CGTK_publication_destroy(publication_t* publication) {
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
	
	CGTK_send_gui_file_progress(messaging, true, publication->progress, publication->path);
}

static void CGTK_publication_finish(void* cls) {
	publication_t* publication = (publication_t*) cls;
	
	CGTK_send_gui_file_complete(messaging, true, publication->path, publication->uri);
	
	GNUNET_CONTAINER_DLL_remove(session.publications_head, session.publications_tail, publication);
	
	CGTK_publication_destroy(publication);
}
