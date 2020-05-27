//
// Created by thejackimonster on 26.05.20.
//

#include "file.h"

#include <unistd.h>
#include <linux/limits.h>
#include <libgen.h>
#include <pwd.h>
#include <string.h>
#include <sys/stat.h>

static void CGTK_config_default(config_t* config) {
	memset(config, 0, sizeof(config_t));
	
	config->autosave = 1;
	
	strncpy(config->nick, getenv("USER\0"), CGTK_NAME_BUFFER_SIZE);
	config->nick[CGTK_NAME_BUFFER_SIZE - 1] = '\0';
}

static const char* CGTK_config_path() {
	struct passwd* pw = getpwuid(getuid());
	const char* home = pw->pw_dir;
	size_t home_len = strlen(home);
	
	static char path [PATH_MAX];
	
	for (size_t i = 0; i < home_len; i++) {
		path[i] = home[i];
	}
	
	strncpy(path + home_len, CGTK_CONFIG_PATH, PATH_MAX - home_len);
	path[PATH_MAX - 1] = '\0';
	
	return path;
}

uint8_t CGTK_config_load(config_t* config) {
	CGTK_config_default(config);
	
	const char* path = CGTK_config_path();
	
	json_t* json = json_load_file(path, 0, NULL);
	
	if (json) {
		json_t* autosave = json_object_get(json, "autosave\0");
		
		if (json_is_boolean(autosave)) {
			config->autosave = json_boolean_value(autosave);
		}
		
		json_t* port = json_object_get(json, "port\0");
		
		if (json_is_string(port)) {
			strncpy(config->port, json_string_value(port), CGTK_NAME_BUFFER_SIZE);
			config->port[CGTK_NAME_BUFFER_SIZE - 1] = '\0';
		}
		
		json_t* nick = json_object_get(json, "nick\0");
		
		if (json_is_string(nick)) {
			strncpy(config->nick, json_string_value(nick), CGTK_NAME_BUFFER_SIZE);
			config->nick[CGTK_NAME_BUFFER_SIZE - 1] = '\0';
		}
		
		json_t* email = json_object_get(json, "email\0");
		
		if (json_is_string(email)) {
			strncpy(config->email, json_string_value(email), CGTK_NAME_BUFFER_SIZE);
			config->email[CGTK_NAME_BUFFER_SIZE - 1] = '\0';
		}
		
		json_t* phone = json_object_get(json, "phone\0");
		
		if (json_is_string(phone)) {
			strncpy(config->phone, json_string_value(phone), CGTK_NAME_BUFFER_SIZE);
			config->phone[CGTK_NAME_BUFFER_SIZE - 1] = '\0';
		}
		
		json_delete(json);
		
		return 1;
	} else {
		return 0;
	}
}

uint8_t CGTK_config_save(const config_t* config) {
	json_t* json = json_object();
	
	json_object_set(json, "autosave\0", json_boolean(config->autosave));
	
	json_object_set(json, "port\0", json_string(config->port));
	
	json_object_set(json, "nick\0", json_string(config->nick));
	json_object_set(json, "email\0", json_string(config->email));
	json_object_set(json, "phone\0", json_string(config->phone));
	
	const char* path = CGTK_config_path();
	char directory [PATH_MAX];
	
	strncpy(directory, path, PATH_MAX);
	
	const char* dir = dirname(directory);
	
	struct stat stats;
	
	if ((stat(dir, &stats) != 0) || (!S_ISDIR(stats.st_mode))) {
		mkdir(dir, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
	}
	
	int dump_code = json_dump_file(json, path, 0);
	
	json_delete(json);
	
	return (dump_code == 0);
}

uint8_t CGTK_config_update(const config_t* new_config, config_t* old_config) {
	int cmp_value = memcmp(new_config, old_config, sizeof(config_t));
	
	if (cmp_value) {
		memcpy(old_config, new_config, sizeof(config_t));
		
		if (new_config->autosave) {
			return CGTK_config_save(new_config);
		}
	}
	
	return 0;
}
