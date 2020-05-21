//
// Created by thejackimonster on 17.04.20.
//

#ifndef CADET_GTK_CONFIG_H
#define CADET_GTK_CONFIG_H

#define CGTK_BINARY_NAME "cadet-gtk\0"
#define CGTK_APPLICATION_ID "com.gitlab.thejackimonster.CADET-GTK\0"
#define CGTK_DESCRIPTION "A GTK based GUI for the CADET subsystem of GNUnet.\0"

/*
 * Delay of every call of CGTK_idle(...) in both processes:
 *  -    0ms: Both processes will drain 100% of a CPU core each.
 *  -    1ms: Best performance without CPU getting drained.
 *  - 1000ms: Actual performance of the CLI application.
 */
#define CGTK_SESSION_IDLE_DELAY_MS 1

/*
 * Delay to reannounce the set name_regex of the user.
 */
#define CGTK_SESSION_ANNOUNCE_DELAY_MIN 5

/*
 * You can assign each process its own delay separate if you want.
 */
#define CGTK_GTK_SESSION_IDLE_DELAY_MS CGTK_SESSION_IDLE_DELAY_MS
#define CGTK_GNUNET_SESSION_IDLE_DELAY_MS  CGTK_SESSION_IDLE_DELAY_MS

#define CGTK_GNUNET_SESSION_ANNOUNCE_DELAY_MIN CGTK_SESSION_ANNOUNCE_DELAY_MIN

/*
 * The compression value will set the maximum length of a path element in
 * the resulting DFA after conversion from the regex:
 *  -    0: Maximum possible compression (generally not desireable)
 *  -    1: No compression at all
 *  -    6: General length for common short nicknames
 */
#define CGTK_NAME_REGEX_COMPRESSION 6

#define CGTK_NAME_SEARCH_PREFIX "cadet-gtk://\0"
#define CGTK_NAME_SEARCH_PREFIX_SIZE 12

#define CGTK_NAME_SEARCH_PREFIX_REG CGTK_NAME_SEARCH_PREFIX
#define CGTK_NAME_SEARCH_PREFIX_REG_SIZE CGTK_NAME_SEARCH_PREFIX_SIZE

#define CGTK_IDENTITY_BUFFER_SIZE 1024
#define CGTK_PORT_BUFFER_SIZE 512
#define CGTK_NAME_SEARCH_SIZE 128
#define CGTK_MESSAGE_BUFFER_SIZE 60000

#endif //CADET_GTK_CONFIG_H
