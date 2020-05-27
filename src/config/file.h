//
// Created by thejackimonster on 26.05.20.
//

#ifndef CADET_GTK_CONFIG_FILE_H
#define CADET_GTK_CONFIG_FILE_H

#include "../config.h"

#include <jansson.h>

uint8_t CGTK_config_load(config_t* config);

uint8_t CGTK_config_save(const config_t* config);

uint8_t CGTK_config_update(const config_t* new_config, config_t* old_config);

#endif //CADET_GTK_CONFIG_FILE_H
