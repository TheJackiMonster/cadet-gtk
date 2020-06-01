//
// Created by thejackimonster on 14.04.20.
//

#ifndef CADET_GTK_GNUNET_H
#define CADET_GTK_GNUNET_H

/** @defgroup gnunet_group GNUnet
 *  Handling the functionality with the GNUnet framework.
 *  @{
 */

#include <gnunet/platform.h>
#include <gnunet/gnunet_util_lib.h>

/**
 * Starts a session using GNUnet and handles all modules and messages respectively
 * receiving via the shared messaging_t struct.
 *
 * @param cls A pointer to a valid messaging_t struct (non-null)
 * @param args Original arguments from main process
 * @param cfgfile Path of the config file for GNUnet
 * @param cfg Configuration handle for GNUnet (non-null)
 */
void CGTK_run_gnunet(void* cls, char*const* args, const char* cfgfile, const struct GNUNET_CONFIGURATION_Handle* cfg);

/** } */

#endif //CADET_GTK_GNUNET_H
