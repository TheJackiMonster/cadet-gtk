//
// Created by thejackimonster on 13.04.20.
//

#ifndef CADET_GTK_MESSAGING_H
#define CADET_GTK_MESSAGING_H

#include <gnunet/gnunet_config.h>
#include <gnunet/gnunet_cadet_service.h>

void* cadet_gtk_on_connect(void* cls, struct GNUNET_CADET_Channel* channel, const struct GNUNET_PeerIdentity* source);

void cadet_gtk_on_disconnect(void* cls, const struct GNUNET_CADET_Channel* channel);

void cadet_gtk_on_window_size_change(void* cls, const struct GNUNET_CADET_Channel* channel, int window_size);

#endif //CADET_GTK_MESSAGING_H
