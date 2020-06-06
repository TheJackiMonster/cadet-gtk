//
// Created by thejackimonster on 26.05.20.
//

#ifndef CADET_GTK_CONFIG_INTERNAL_H
#define CADET_GTK_CONFIG_INTERNAL_H

#include <gnunet/gnunet_constants.h>

/*
 * Only necessary for debug builds if something needs to get found by logging every call.
 */
#ifndef NDEBUG
//#define CGTK_ALL_DEBUG
#endif

#define CGTK_BINARY_NAME "cadet-gtk\0"
#define CGTK_APPLICATION_ID "com.gitlab.thejackimonster.CADET-GTK\0"
#define CGTK_APPLICATION_NAME "Cadet-GTK\0"
#define CGTK_DESCRIPTION "A GTK based GUI for the CADET subsystem of GNUnet.\0"
#define CGTK_CONFIG_PATH "/.config/cadet-gtk/config.json\0"
#define CGTK_STORAGE_PATH "/.local/share/cadet-gtk\0"

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

/*
 * The prefix for the regex does not have to differentiate
 * until it contains characters which need to be escaped.
 */
#define CGTK_NAME_SEARCH_PREFIX_REG CGTK_NAME_SEARCH_PREFIX
#define CGTK_NAME_SEARCH_PREFIX_REG_SIZE CGTK_NAME_SEARCH_PREFIX_SIZE

#define CGTK_FILE_EXTENSION_MAX_ESTIMATE 15

#define CGTK_IDENTITY_BUFFER_SIZE 1024
#define CGTK_PORT_BUFFER_SIZE 512
#define CGTK_NAME_BUFFER_SIZE 128
#define CGTK_REGEX_BUFFER_SIZE 1024
#define CGTK_MESSAGE_BUFFER_SIZE GNUNET_CONSTANTS_MAX_CADET_MESSAGE_SIZE

#define CGTK_VISIBILITY_PUBLIC 0
#define CGTK_VISIBILITY_PUBLIC_ID "public\0"
#define CGTK_VISIBILITY_PRIVATE 1
#define CGTK_VISIBILITY_PRIVATE_ID "private\0"
#define CGTK_VISIBILITY_CAT 2
#define CGTK_VISIBILITY_CAT_ID "cat\0"

#define CGTK_ANIMATION_DURATION 100

#endif //CADET_GTK_CONFIG_INTERNAL_H
