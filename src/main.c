//
// Created by thejackimonster on 12.04.20.
//

#include <pthread.h>

#include <gnunet/platform.h>
#include <gnunet/gnunet_util_lib.h>

#include "messaging.h"
#include "handy_ui.h"

static struct {
	pthread_mutex_t mutex;
	bool alive;
	const struct GNUNET_CONFIGURATION_Handle* cfg;
	struct GNUNET_CADET_Handle* cadet;
	struct GNUNET_CADET_Port* listen;
} session;

static void CGTK_shutdown(void* cls) {
	if (session.listen) {
		GNUNET_CADET_close_port(session.listen);
	}
	
	GNUNET_CADET_disconnect(session.cadet);
	
	printf("shutdown!\n");
}

static void handle_message(void* cls, const struct GNUNET_MessageHeader* message) {
	//
	
	//GNUNET_CADET_receive_done();
	
	printf("message: !\n");
}

static void CGTK_run(void* cls, char*const* args, const char* cfgfile, const struct GNUNET_CONFIGURATION_Handle* cfg) {
	messaging_t* messaging = (messaging_t*) cls;
	
	session.cfg = cfg;
	session.cadet = GNUNET_CADET_connect(cfg);
	
	if (!session.cadet) {
		GNUNET_SCHEDULER_shutdown();
		return;
	}
	
	GNUNET_SCHEDULER_add_shutdown(&CGTK_shutdown, NULL);
	
	const char* port_s = "test\0";
	const char* port_us = GNUNET_STRINGS_to_utf8(port_s, strlen(port_s), "ASCII");
	
	struct GNUNET_HashCode port;
	GNUNET_CRYPTO_hash(port_us, strlen(port_us), &port);
	
	struct GNUNET_MQ_MessageHandler handlers[] = {
			GNUNET_MQ_hd_fixed_size(
					message,
					GNUNET_MESSAGE_TYPE_CADET_CLI,
					struct GNUNET_MessageHeader,
					NULL
			), GNUNET_MQ_handler_end()
	};
	
	struct GNUNET_PeerIdentity peer;
	
	if (GNUNET_CRYPTO_get_peer_identity(cfg, &peer) != GNUNET_OK) {
		memset(&peer, 0, sizeof(peer));
	}
	
	CGTK_send_gtk_identity(messaging, &peer);
	
	session.listen = GNUNET_CADET_open_port(
			session.cadet,
			&port,
			&CGTK_on_connect,
			NULL,
			&CGTK_on_window_size_change,
			&CGTK_on_disconnect,
			handlers
	);
}

static void* CGTK_poll(void* args) {
	messaging_t* messaging = (messaging_t*) args;
	
	GtkWidget* window = GTK_WIDGET(gtk_window_list_toplevels()->data);
	
	while (TRUE) {
		pthread_mutex_lock(&(session.mutex));
		
		if (!session.alive) {
			break;
		}
		
		switch (CGTK_recv_gnunet_msg_type(messaging)) {
			case GTK_IDENTITY:
				CGTK_update_identity_ui(window, CGTK_recv_gnunet_identity(messaging));
				break;
			default:
				break;
		}
		
		pthread_mutex_unlock(&(session.mutex));
	}
	
	return NULL;
}

static void CGTK_end_thread(GtkWidget* window, gpointer user_data) {
	pthread_t* msg_thread = (pthread_t*) user_data;
	
	pthread_mutex_lock(&(session.mutex));
	session.alive = FALSE;
	pthread_mutex_unlock(&(session.mutex));
	
	pthread_join(*msg_thread, NULL);
}

static void CGTK_activate(GtkApplication* application, gpointer user_data) {
	messaging_t* messaging = (messaging_t*) user_data;
	
	GtkWidget* window = gtk_application_window_new(application);
	gtk_window_set_default_size(GTK_WINDOW(window), 320, 512);
	
	CGTK_init_ui(window);
	
	pthread_t msg_thread;
	
	pthread_create(&msg_thread, NULL, &CGTK_poll, messaging);
	
	g_signal_connect(window, "destroy", G_CALLBACK(CGTK_end_thread), &msg_thread);
	
	gtk_widget_show_all(window);
}

int main(int argc, char** argv) {
	messaging_t messaging;
	
	CGTK_init_messaging(&messaging);
	
	pthread_mutexattr_t attr;
	
	if ((pthread_mutexattr_init(&attr) == -1) ||
		(pthread_mutex_init(&(session.mutex), &attr) == -1)) {
		exit(EXIT_FAILURE);
	}
	
	pthread_mutex_lock(&(session.mutex));
	session.alive = TRUE;
	pthread_mutex_unlock(&(session.mutex));
	
	pid_t pid = fork();
	
	if (pid == 0) {
		struct GNUNET_GETOPT_CommandLineOption options[] = {
				GNUNET_GETOPT_OPTION_END
		};
		
		CGTK_prepare_gnunet(&messaging);
		
		int status = (GNUNET_OK == GNUNET_PROGRAM_run(
				argc, argv,
				"cadet-gtk",
				gettext_noop("A GTK based GUI for the CADET subsystem of GNUnet."),
				options, &CGTK_run, &messaging
		)? EXIT_SUCCESS : EXIT_FAILURE);
		
		printf("net exit\n");
		
		CGTK_close_messaging(&messaging);
		
		return status;
	} else {
		gtk_init(&argc, &argv);
		
		GtkApplication* application = gtk_application_new(
				"org.gnunet.CADET",
				G_APPLICATION_FLAGS_NONE
		);
		
		g_signal_connect(application, "activate", G_CALLBACK(CGTK_activate), &messaging);
		
		CGTK_prepare_gtk(&messaging);
		
		int status = g_application_run(G_APPLICATION(application), argc, argv);
		
		printf("app exit\n");
		
		if (pid > 0) {
			kill(pid, SIGTERM);
			
			waitpid(pid, NULL, NULL);
		}
		
		g_object_unref(application);
		
		pthread_mutex_destroy(&(session.mutex));
		
		CGTK_close_messaging(&messaging);
		
		return status;
	}
}
