//
// Created by thejackimonster on 17.04.20.
//

#ifndef CADET_GTK_CONFIG_H
#define CADET_GTK_CONFIG_H

#include <stdint.h>

#include "config/internal.h"

typedef struct config_t {
	uint8_t autosave;
	
	char nick [CGTK_NAME_BUFFER_SIZE];
	char email [CGTK_NAME_BUFFER_SIZE];
	char phone [CGTK_NAME_BUFFER_SIZE];
	
	uint8_t regex_attr;
	
	uint8_t visibility;
	char port [CGTK_PORT_BUFFER_SIZE];
} config_t;

#include "config/file.h"

#endif //CADET_GTK_CONFIG_H
