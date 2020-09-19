//
// Created by thejackimonster on 13.04.20.
//

#include "messaging.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <fcntl.h>
#include <unistd.h>

static struct GNUNET_CONTAINER_MultiHashMap* port_lookup;

static int __pipe2(int pipefd[2], int flags) {
	int result = pipe(pipefd);
	
	if (result == 0) {
		result = fcntl(pipefd[0], F_SETFL, flags) | fcntl(pipefd[1], F_SETFL, flags);
	}
	
	return result;
}

void CGTK_init_messaging(messaging_t* messaging) {
	if ((__pipe2(messaging->pipe_gnunet, O_NONBLOCK) == -1) ||
		(__pipe2(messaging->pipe_gui, O_NONBLOCK) == -1)) {
		exit(EXIT_FAILURE);
	}
	
	if (!port_lookup) {
		port_lookup = GNUNET_CONTAINER_multihashmap_create(8, GNUNET_NO);
		
		char* empty_string = GNUNET_malloc(sizeof(char));
		empty_string[0] = '\0';
		
		struct GNUNET_HashCode hashcode;
		GNUNET_CRYPTO_hash(NULL, 0, &hashcode);
		GNUNET_CONTAINER_multihashmap_put(
				port_lookup,
				&hashcode,
				empty_string,
				GNUNET_CONTAINER_MULTIHASHMAPOPTION_UNIQUE_ONLY
		);
	}
}

static void close_if_open(int* fd) {
	if (*fd) {
		close(*fd);
		*fd = 0;
	}
}

#include "messaging/gui.c"
#include "messaging/gnunet.c"

void CGTK_close_messaging(messaging_t* messaging) {
	close_if_open(&(messaging->pipe_gnunet[0]));
	close_if_open(&(messaging->pipe_gnunet[1]));
	close_if_open(&(messaging->pipe_gui[0]));
	close_if_open(&(messaging->pipe_gui[1]));
}

static int CGTK_clear_lookup(void *cls, const struct GNUNET_HashCode* key, void* value) {
	GNUNET_free(value);
	return GNUNET_YES;
}

void CGTK_shutdown_messaging(void) {
	if (port_lookup) {
		GNUNET_CONTAINER_multihashmap_iterate(port_lookup, CGTK_clear_lookup, NULL);
		GNUNET_CONTAINER_multihashmap_destroy(port_lookup);
		port_lookup = NULL;
	}
}
