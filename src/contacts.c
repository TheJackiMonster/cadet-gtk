//
// Created by thejackimonster on 12.04.20.
//

#include "contacts.h"

void cadet_gtk_init_contacts(GtkWidget* header, GtkWidget* content) {
	gtk_header_bar_set_title(GTK_HEADER_BAR(header), "Contacts");
	gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header), TRUE);
	gtk_header_bar_set_has_subtitle(GTK_HEADER_BAR(header), FALSE);
	
	GtkSizeGroup* sizeGroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	gtk_size_group_add_widget(sizeGroup, header);
	gtk_size_group_add_widget(sizeGroup, content);
	
	//
}
