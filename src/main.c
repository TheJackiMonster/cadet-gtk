//
// Created by thejackimonster on 12.04.20.
//

#include "gtk.h"
#include "gnunet.h"

#include "config.h"
#include "messaging.h"

int main(int argc, char** argv) {
	messaging_t messaging;
	
	CGTK_init_messaging(&messaging);
	
	pid_t pid = fork();
	
	if (pid == 0) {
		struct GNUNET_GETOPT_CommandLineOption options[] = {
				GNUNET_GETOPT_OPTION_END
		};
		
		CGTK_prepare_gnunet(&messaging);
		
		int status = (GNUNET_OK == GNUNET_PROGRAM_run(
				argc, argv,
				CGTK_BINARY_NAME,
				gettext_noop(CGTK_DESCRIPTION),
				options, &CGTK_run_gnunet, &messaging
		)? EXIT_SUCCESS : EXIT_FAILURE);
		
		CGTK_close_messaging(&messaging);
		CGTK_shutdown_messaging();
		
		return status;
	} else {
		gtk_init(&argc, &argv);
		
		GtkApplication* application = gtk_application_new(
				CGTK_APPLICATION_ID,
				G_APPLICATION_NON_UNIQUE
		);
		
		g_signal_connect(application, "activate\0", G_CALLBACK(CGTK_activate_gtk), &messaging);
		
		CGTK_prepare_gui(&messaging);
		
		int status = g_application_run(G_APPLICATION(application), argc, argv);
		
		if (pid > 0) {
			kill(pid, SIGTERM);
			
			waitpid(pid, NULL, 0);
		}
		
		g_object_unref(application);
		
		CGTK_close_messaging(&messaging);
		CGTK_shutdown_messaging();
		
		return status;
	}
}
