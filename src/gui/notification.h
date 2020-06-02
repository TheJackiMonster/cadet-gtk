//
// Created by thejackimonster on 02.06.20.
//

#ifndef CADET_GTK_NOTIFICATION_H
#define CADET_GTK_NOTIFICATION_H

/** @addtogroup gtk_group
 *  @{
 */

#include <libnotify/notify.h>

#include "../gui.h"
#include "util.h"

void CGTK_notification_from_chat(cgtk_gui_t* gui, const char* identity, const char* port, const msg_t* msg);

/** } */

#endif //CADET_GTK_NOTIFICATION_H
