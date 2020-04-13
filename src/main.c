//
// Created by thejackimonster on 12.04.20.
//

#include <semaphore.h>

#include <gnunet/platform.h>
#include <gnunet/gnunet_mq_lib.h>
#include <gnunet/gnunet_util_lib.h>

#include "handy_ui.h"
#include "messaging.h"

typedef struct {
	bool disconnect;
	pthread_mutex_t mutex;
} shared_data;

static shared_data* data = NULL;

void initialise_shared() {
	data = mmap(
			NULL, sizeof(shared_data),
			PROT_READ | PROT_WRITE,
			MAP_SHARED | MAP_ANONYMOUS,
			-1, 0
	);
	
	g_assert(data);
	
	data->disconnect = FALSE;
	
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
	
	pthread_mutex_init(&(data->mutex), &attr);
	pthread_mutex_lock(&(data->mutex));
}

static void handle_cli_message(void* cls, const struct GNUNET_MessageHeader* message) {
	//
	
	//GNUNET_CADET_receive_done();
	
	printf("message: !\n");
}

static void run (void* cls, char*const* args, const char* cfgfile, const struct GNUNET_CONFIGURATION_Handle* cfg) {
	struct GNUNET_CADET_Handle* cadet = GNUNET_CADET_connect(cfg);
	
	const char* port_s = "test\0";
	const char* port_us = GNUNET_STRINGS_to_utf8(port_s, strlen(port_s), "ASCII");
	
	struct GNUNET_HashCode port;
	GNUNET_CRYPTO_hash(port_us, strlen(port_us), &port);
	
	struct GNUNET_MQ_MessageHandler handlers[] = {
			GNUNET_MQ_hd_fixed_size(
					cli_message,
					GNUNET_MESSAGE_TYPE_CADET_CLI,
					struct GNUNET_MessageHeader,
					NULL
			), GNUNET_MQ_handler_end()
	};
	
	struct GNUNET_PeerIdentity peer;
	
	if (GNUNET_CRYPTO_get_peer_identity(cfg, &peer) == GNUNET_OK) {
		const struct GNUNET_PeerIdentity* const_peer = &peer;
		const char* str_info = GNUNET_i2s_full(const_peer);
		
		printf("peer: %s\n", str_info);
	}
	
	struct GNUNET_CADET_Port* open_port = GNUNET_CADET_open_port(
			cadet,
			&port,
			&cadet_gtk_on_connect,
			NULL,
			NULL,
			&cadet_gtk_on_disconnect,
			handlers
	);
	
	while (TRUE) {
		pthread_mutex_lock(&(data->mutex));
		
		if (data->disconnect) {
			break;
		}
		
		pthread_mutex_unlock(&(data->mutex));
	}
	
	if (open_port) {
		GNUNET_CADET_close_port(open_port);
	}
	
	GNUNET_CADET_disconnect(cadet);
}

static void activate(GtkApplication* application, gpointer user_data) {
	GtkWidget* window = gtk_application_window_new(application);
	gtk_window_set_default_size(GTK_WINDOW(window), 320, 512);
	
	cadet_gtk_init_ui(window);
	
	pthread_mutex_unlock(&(data->mutex));
	
	gtk_widget_show_all(window);
}

int main(int argc, char** argv) {
	initialise_shared();
	
	pid_t pid = fork();
	
	if (pid == 0) {
		struct GNUNET_GETOPT_CommandLineOption options[] = {
				GNUNET_GETOPT_OPTION_END
		};
		
		int status = (GNUNET_OK == GNUNET_PROGRAM_run(
				argc, argv,
				"cadet-gtk",
				gettext_noop("A GTK based GUI for the CADET subsystem of GNUnet."),
				options, &run, NULL
		)? 0 : 1);
		
		return status;
	} else {
		gtk_init(&argc, &argv);
		
		GtkApplication* application = gtk_application_new(
				"org.gnunet.CADET",
				G_APPLICATION_FLAGS_NONE
		);
		
		g_signal_connect(application, "activate", G_CALLBACK(activate), NULL);
		
		int status = g_application_run(G_APPLICATION(application), argc, argv);
		
		data->disconnect = TRUE;
		
		if (pid > 0) {
			waitpid(pid, NULL, NULL);
		}
		
		pthread_mutex_destroy(&(data->mutex));
		
		g_object_unref(application);
		
		munmap(data, sizeof(shared_data));
		
		return status;
	}
}
