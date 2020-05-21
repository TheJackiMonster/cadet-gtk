//
// Created by thejackimonster on 30.04.20.
//

#include "json.h"

#include <string.h>
#include <time.h>

const char* CGTK_encode_message(const msg_t* msg, size_t* message_len) {
	json_t* json = json_object();
	
	switch (msg->kind) {
		case MSG_KIND_TALK: {
			json_object_set(json, "kind\0", json_string("talk\0"));
			break;
		} case MSG_KIND_JOIN: {
			json_object_set(json, "kind\0", json_string("join\0"));
			break;
		} case MSG_KIND_LEAVE: {
			json_object_set(json, "kind\0", json_string("leave\0"));
			break;
		} case MSG_KIND_INFO: {
			json_object_set(json, "kind\0", json_string("info\0"));
			break;
		} default: {
			break;
		}
	}
	
	json_object_set(json, "timestamp\0", json_integer(msg->timestamp));
	
	if (msg->sender) {
		json_object_set(json, "sender\0", json_string(msg->sender));
	}
	
	if (msg->content) {
		json_object_set(json, "content\0", json_string(msg->content));
	}
	
	if (msg->who) {
		json_object_set(json, "who\0", json_string(msg->who));
	}
	
	if (msg->participants) {
		json_t* participants = json_array();
		const char** part = msg->participants;
		
		while (*part) {
			json_array_append(participants, json_string(*part));
			part++;
		}
		
		json_object_set(json, "participants\0", participants);
	}
	
	const char* message = json_dumps(json, JSON_COMPACT);
	
	json_delete(json);
	
	if (message) {
		*message_len = strlen(message);
	} else {
		*message_len = 0;
	}
	
	return message;
}

static const char* CGTK_string_clone(const char* string) {
	if (string) {
		const size_t len = strlen(string);
		char* clone = (char*) malloc(len + 1);
		
		strcpy(clone, string);
		
		clone[len] = '\0';
		return clone;
	} else {
		return string;
	}
}

msg_t* CGTK_decode_message(const char* message, size_t message_len) {
	msg_t* msg = (msg_t*) malloc(sizeof(msg_t));
	
	memset(msg, 0, sizeof(msg_t));
	
	json_t* json = json_loadb(message, message_len, 0, NULL);
	
	if (json) {
		json_t* kind = json_object_get(json, "kind\0");
		json_t* timestamp = json_object_get(json, "timestamp\0");
		
		json_t* sender = json_object_get(json, "sender\0");
		json_t* content = json_object_get(json, "content\0");
		
		json_t* who = json_object_get(json, "who\0");
		
		json_t* participants = json_object_get(json, "participants\0");
		
		if (kind) {
			const char* kind_str = json_string_value(kind);
			
			msg->decoding |= MSG_DEC_KIND_BIT;
			
			if (strcmp(kind_str, "talk\0") == 0) {
				msg->kind = MSG_KIND_TALK;
			} else
			if (strcmp(kind_str, "join\0") == 0) {
				msg->kind = MSG_KIND_JOIN;
			} else
			if (strcmp(kind_str, "leave\0") == 0) {
				msg->kind = MSG_KIND_LEAVE;
			} else
			if (strcmp(kind_str, "info\0") == 0) {
				msg->kind = MSG_KIND_INFO;
			} else {
				msg->decoding ^= MSG_DEC_KIND_BIT;
			}
		}
		
		if (timestamp) {
			msg->timestamp = (time_t) json_integer_value(timestamp);
			
			msg->decoding |= MSG_DEC_TIMESTAMP_BIT;
		}
		
		if (sender) {
			msg->sender = CGTK_string_clone(json_string_value(sender));
			
			msg->decoding |= MSG_DEC_SENDER_BIT;
		}
		
		if (content) {
			msg->content = CGTK_string_clone(json_string_value(content));
			
			msg->decoding |= MSG_DEC_CONTENT_BIT;
		}
		
		if (who) {
			msg->who = CGTK_string_clone(json_string_value(who));
			
			msg->decoding |= MSG_DEC_WHO_BIT;
		}
		
		if (participants) {
			size_t count = json_array_size(participants);
			
			msg->participants = (const char**) malloc((count + 1) * sizeof(const char*));
			
			for (size_t i = 0; i < count; i++) {
				json_t* part = json_array_get(participants, i);
				
				msg->participants[i] = CGTK_string_clone(json_string_value(part));
			}
			
			msg->participants[count] = NULL;
			msg->decoding |= MSG_DEC_PARTICIPANTS_BIT;
		}
		
		json_delete(json);
	}
	
	return msg;
}

void CGTK_repair_message(msg_t* msg, const char* message, const char* sender) {
	if (!(msg->decoding & MSG_DEC_KIND_BIT)) {
		msg->kind = MSG_KIND_TALK;
	}
	
	if (!(msg->decoding & MSG_DEC_TIMESTAMP_BIT)) {
		msg->timestamp = time(NULL);
	}
	
	if (msg->kind == MSG_KIND_TALK) {
		if (!(msg->decoding & MSG_DEC_SENDER_BIT)) {
			msg->sender = sender;
		}
		
		if (msg->decoding == 0) {
			msg->content = message;
		} else
		if (!(msg->decoding & MSG_DEC_CONTENT_BIT)) {
			msg->content = "\0";
		}
	}
}

void CGTK_free_message(msg_t* msg) {
	if (msg->decoding & MSG_DEC_SENDER_BIT) {
		free((void*) msg->sender);
	}
	
	if (msg->decoding & MSG_DEC_CONTENT_BIT) {
		free((void*) msg->content);
	}
	
	if (msg->decoding & MSG_DEC_WHO_BIT) {
		free((void*) msg->who);
	}
	
	if (msg->decoding & MSG_DEC_PARTICIPANTS_BIT) {
		const char** part = msg->participants;
		
		while (*part) {
			free((void*) (*part));
			part++;
		}
		
		free((void*) msg->participants);
	}
	
	free(msg);
}
