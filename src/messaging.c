//
// Created by thejackimonster on 13.04.20.
//

#include "messaging.h"

void* cadet_gtk_on_connect(void* cls, struct GNUNET_CADET_Channel* channel, const struct GNUNET_PeerIdentity* source) {
	// TODO
	
	printf("connect: %s\n", (const char*) source->public_key.q_y);
	
	return NULL;
}


void cadet_gtk_on_disconnect(void* cls, const struct GNUNET_CADET_Channel* channel) {
	// TODO
	
	printf("disconnect!\n");
}

void cadet_gtk_on_window_size_change(void* cls, const struct GNUNET_CADET_Channel* channel, int window_size) {
	// TODO
	
	printf("window_size_change: %d\n", window_size);
}
