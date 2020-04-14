//
// Created by thejackimonster on 12.04.20.
//

#include "gtk.h"
#include "gnunet.h"
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
				"cadet-gtk",
				gettext_noop("A GTK based GUI for the CADET subsystem of GNUnet."),
				options, &CGTK_run, &messaging
		)? EXIT_SUCCESS : EXIT_FAILURE);
		
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
		
		if (pid > 0) {
			kill(pid, SIGTERM);
			
			waitpid(pid, NULL, NULL);
		}
		
		g_object_unref(application);
		
		CGTK_close_messaging(&messaging);
		
		return status;
	}
}
