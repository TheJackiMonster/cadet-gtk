//
// Created by thejackimonster on 14.04.20.
//

#ifndef CADET_GTK_GTK_H
#define CADET_GTK_GTK_H

/** @defgroup gtk_group GTK
 *  Handling the GUI implemented in GTK.
 *  @{
 */

#include <gtk/gtk.h>
#include <libnotify/notify.h>

/**
 * Starts the application on activation in GTK and creates a GUI
 * respectively to all client-side required functionality.
 *
 * @param application A pointer to the current GTKApplication (non-null)
 * @param user_data A pointer to a valid messaging_t struct (non-null)
 */
void CGTK_activate_gtk(GtkApplication* application, gpointer user_data);

/** } */

#endif //CADET_GTK_GTK_H
