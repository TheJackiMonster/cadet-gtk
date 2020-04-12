//
// Created by thejackimonster on 12.04.20.
//

#include "handy_ui.h"

#include <gnunet/gnunet_config.h>
#include <gnunet/gnunet_cadet_service.h>

static void activate(GtkApplication* application, gpointer user_data) {
	GtkWidget* window = gtk_application_window_new(application);
	
	cadet_gtk_init_ui(window);
	
	gtk_window_set_default_size(GTK_WINDOW(window), 320, 512);
	gtk_widget_show_all(window);
}

int main(int argc, char** argv) {
	gtk_init(&argc, &argv);
	
	GtkApplication* application = gtk_application_new(
			"org.gnunet.CADET",
			G_APPLICATION_FLAGS_NONE
	);
	
	g_signal_connect(application, "activate", G_CALLBACK(activate), NULL);
	
	int status = g_application_run(G_APPLICATION(application), argc, argv);
	
	g_object_unref(application);
	
	return 0;
}
