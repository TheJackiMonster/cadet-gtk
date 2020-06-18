//
// Created by thejackimonster on 30.04.20.
//

#include "msg.h"

#include <string.h>
#include <time.h>

const char* CGTK_encode_message(const msg_t* msg, size_t* message_len) {
#ifdef CGTK_ALL_DEBUG
	printf("MSG: CGTK_encode_message()\n");
#endif
	
	json_t* json = json_object();
	
	switch (msg->kind) {
		case MSG_KIND_TALK: {
			json_object_set(json, "kind\0", json_string("talk\0"));
			
			if (msg->talk.sender) {
				json_object_set(json, "sender\0", json_string(msg->talk.sender));
			}
			
			if (msg->talk.content) {
				json_object_set(json, "content\0", json_string(msg->talk.content));
			}
			
			break;
		} case MSG_KIND_JOIN: {
			json_object_set(json, "kind\0", json_string("join\0"));
			
			if (msg->join_leave.who) {
				json_object_set(json, "who\0", json_string(msg->join_leave.who));
			}
			
			break;
		} case MSG_KIND_LEAVE: {
			json_object_set(json, "kind\0", json_string("leave\0"));
			
			if (msg->join_leave.who) {
				json_object_set(json, "who\0", json_string(msg->join_leave.who));
			}
			
			break;
		} case MSG_KIND_INFO: {
			json_object_set(json, "kind\0", json_string("info\0"));
			
			if (msg->info.participants) {
				json_t* participants = json_array();
				const char** part = msg->info.participants;
				
				while (*part) {
					json_array_append(participants, json_string(*part));
					part++;
				}
				
				json_object_set(json, "participants\0", participants);
			}
			
			break;
		} case MSG_KIND_FILE: {
			json_object_set(json, "kind\0", json_string("file\0"));
			
			if (msg->file.publisher) {
				json_object_set(json, "publisher\0", json_string(msg->file.publisher));
			}
			
			if (msg->file.uri) {
				json_object_set(json, "uri\0", json_string(msg->file.uri));
			}
			
			if (msg->file.hash) {
				json_object_set(json, "hash\0", json_string(msg->file.hash));
			}
			
			if (msg->file.name) {
				json_object_set(json, "name\0", json_string(msg->file.name));
			}
			
			break;
		} case MSG_KIND_KEY: {
			json_object_set(json, "kind\0", json_string("key\0"));
			
			switch (msg->key.type) {
				case MSG_KEY_1TU: {
					json_object_set(json, "type\0", json_string("one-time-use\0"));
					break;
				} case MSG_KEY_GPG: {
					json_object_set(json, "type\0", json_string("gnu-privacy-guard\0"));
					break;
				} default: {
					break;
				}
			}
			
			if (msg->key.data) {
				json_object_set(json, "data\0", json_string(msg->key.data));
			}
			
			break;
		} default: {
			break;
		}
	}
	
	json_object_set(json, "timestamp\0", json_integer(msg->timestamp));
	
	const char* message = json_dumps(json, JSON_COMPACT);
	
	json_delete(json);
	
	if (message) {
		*message_len = strlen(message);
	} else {
		*message_len = 0;
	}
	
	return message;
}

static const char* CGTK_string_clone_full(const char* string, size_t len) {
#ifdef CGTK_ALL_DEBUG
	printf("MSG: CGTK_string_clone_full()\n");
#endif
	
	char* clone = (char*) malloc(len + 1);
	
	for (size_t i = 0; i < len; i++) {
		clone[i] = string[i];
	}
	
	clone[len] = '\0';
	return clone;
}

static const char* CGTK_string_clone(const char* string) {
	if (string) {
		return CGTK_string_clone_full(string, strlen(string));
	} else {
		return string;
	}
}

msg_t* CGTK_decode_message(const char* message, size_t message_len) {
#ifdef CGTK_ALL_DEBUG
	printf("MSG: CGTK_decode_message()\n");
#endif
	
	msg_t* msg = (msg_t*) malloc(sizeof(msg_t));
	
	memset(msg, 0, sizeof(msg_t));
	
	json_t* json = json_loadb(message, message_len, 0, NULL);
	
	if (json) {
		json_t* kind = json_object_get(json, "kind\0");
		
		if (kind) {
			const char* kind_str = json_string_value(kind);
			
			msg->decoding |= MSG_DEC_KIND_BIT;
			
			if (strcmp(kind_str, "talk\0") == 0) {
				msg->kind = MSG_KIND_TALK;
				
				json_t* sender = json_object_get(json, "sender\0");
				json_t* content = json_object_get(json, "content\0");
				
				if (sender) {
					msg->talk.sender = CGTK_string_clone(json_string_value(sender));
					
					msg->decoding |= MSG_DEC_SENDER_BIT;
				}
				
				if (content) {
					msg->talk.content = CGTK_string_clone(json_string_value(content));
					
					msg->decoding |= MSG_DEC_CONTENT_BIT;
				}
			} else
			if (strcmp(kind_str, "join\0") == 0) {
				msg->kind = MSG_KIND_JOIN;
				
				json_t* who = json_object_get(json, "who\0");
				
				if (who) {
					msg->join_leave.who = CGTK_string_clone(json_string_value(who));
					
					msg->decoding |= MSG_DEC_WHO_BIT;
				}
			} else
			if (strcmp(kind_str, "leave\0") == 0) {
				msg->kind = MSG_KIND_LEAVE;
				
				json_t* who = json_object_get(json, "who\0");
				
				if (who) {
					msg->join_leave.who = CGTK_string_clone(json_string_value(who));
					
					msg->decoding |= MSG_DEC_WHO_BIT;
				}
			} else
			if (strcmp(kind_str, "info\0") == 0) {
				msg->kind = MSG_KIND_INFO;
				
				json_t* participants = json_object_get(json, "participants\0");
				
				if (participants) {
					size_t count = json_array_size(participants);
					
					msg->info.participants = (const char**) malloc((count + 1) * sizeof(const char*));
					
					for (size_t i = 0; i < count; i++) {
						json_t* part = json_array_get(participants, i);
						
						msg->info.participants[i] = CGTK_string_clone(json_string_value(part));
					}
					
					msg->info.participants[count] = NULL;
					msg->decoding |= MSG_DEC_PARTICIPANTS_BIT;
				}
			} else
			if (strcmp(kind_str, "file\0") == 0) {
				msg->kind = MSG_KIND_FILE;
				
				json_t* publisher = json_object_get(json, "publisher\0");
				json_t* uri = json_object_get(json, "uri\0");
				json_t* hash = json_object_get(json, "hash\0");
				json_t* name = json_object_get(json, "name\0");
				
				if (publisher) {
					msg->file.publisher = CGTK_string_clone(json_string_value(publisher));
					
					msg->decoding |= MSG_DEC_PUBLISHER_BIT;
				}
				
				if (uri) {
					msg->file.uri = CGTK_string_clone(json_string_value(uri));
					
					msg->decoding |= MSG_DEC_URI_BIT;
				}
				
				if (hash) {
					msg->file.hash = CGTK_string_clone(json_string_value(hash));
					
					msg->decoding |= MSG_DEC_HASH_BIT;
				}
				
				if (name) {
					msg->file.name = CGTK_string_clone(json_string_value(name));
					
					msg->decoding |= MSG_DEC_NAME_BIT;
				}
			} else
			if (strcmp(kind_str, "key\0") == 0) {
				msg->kind = MSG_KIND_KEY;
				
				json_t* type = json_object_get(json, "type\0");
				json_t* data = json_object_get(json, "data\0");
				
				if (type) {
					const char* key_type_str = json_string_value(type);
					
					msg->decoding |= MSG_DEC_TYPE_BIT;
					
					if (strcmp(key_type_str, "one-time-use\0") == 0) {
						msg->key.type = MSG_KEY_1TU;
					} else
					if (strcmp(key_type_str, "gnu-privacy-guard\0") == 0) {
						msg->key.type = MSG_KEY_GPG;
					} else {
						msg->decoding ^= MSG_DEC_TYPE_BIT;
					}
				}
				
				if (data) {
					msg->key.data = CGTK_string_clone(json_string_value(data));
					
					msg->decoding |= MSG_DEC_DATA_BIT;
				}
			} else {
				msg->decoding ^= MSG_DEC_KIND_BIT;
			}
		}
		
		json_t* timestamp = json_object_get(json, "timestamp\0");
		
		if (timestamp) {
			msg->timestamp = (time_t) json_integer_value(timestamp);
			
			msg->decoding |= MSG_DEC_TIMESTAMP_BIT;
		}
		
		json_delete(json);
	}
	
	msg->decoding &= MSG_DEC_COMPLETE_BITS;
	
	return msg;
}

void CGTK_repair_message(msg_t* msg, const char* message, size_t message_len, const char* sender) {
#ifdef CGTK_ALL_DEBUG
	printf("MSG: CGTK_repair_message()\n");
#endif
	
	if (!(msg->decoding & MSG_DEC_KIND_BIT)) {
		msg->kind = MSG_KIND_TALK;
	}
	
	if (!(msg->decoding & MSG_DEC_TIMESTAMP_BIT)) {
		msg->timestamp = time(NULL);
	}
	
	if (msg->kind == MSG_KIND_TALK) {
		if (!(msg->decoding & MSG_DEC_SENDER_BIT)) {
			msg->talk.sender = sender;
		}
		
		if (msg->decoding == 0) {
			msg->talk.content = CGTK_string_clone_full(message, message_len);
			msg->decoding |= MSG_DEC_CONTENT_BIT;
		} else
		if (!(msg->decoding & MSG_DEC_CONTENT_BIT)) {
			msg->talk.content = "\0";
		}
	}
	
	if (msg->kind == MSG_KIND_FILE) {
		if (!(msg->decoding & MSG_DEC_PUBLISHER_BIT)) {
			msg->file.publisher = sender;
		}
		
		if (!(msg->decoding & MSG_DEC_NAME_BIT)) {
			msg->file.name = "\0";
		}
		
		if (!(msg->decoding & MSG_DEC_HASH_BIT)) {
			msg->file.hash = "\0";
		}
	}
}

void CGTK_free_message(msg_t* msg) {
#ifdef CGTK_ALL_DEBUG
	printf("MSG: CGTK_free_message()\n");
#endif
	
	if (msg->decoding & MSG_DEC_SENDER_BIT) {
		free((void*) msg->talk.sender);
	}
	
	if (msg->decoding & MSG_DEC_CONTENT_BIT) {
		free((void*) msg->talk.content);
	}
	
	if (msg->decoding & MSG_DEC_WHO_BIT) {
		free((void*) msg->join_leave.who);
	}
	
	if (msg->decoding & MSG_DEC_PARTICIPANTS_BIT) {
		const char** part = msg->info.participants;
		
		while (*part) {
			free((void*) (*part));
			part++;
		}
		
		free((void*) msg->info.participants);
	}
	
	if (msg->decoding & MSG_DEC_PUBLISHER_BIT) {
		free((void*) msg->file.publisher);
	}
	
	if (msg->decoding & MSG_DEC_HASH_BIT) {
		free((void*) msg->file.hash);
	}
	
	if (msg->decoding & MSG_DEC_NAME_BIT) {
		free((void*) msg->file.name);
	}
	
	if (msg->decoding & MSG_DEC_URI_BIT) {
		free((void*) msg->file.uri);
	}
	
	if (msg->decoding & MSG_DEC_DATA_BIT) {
		free((void*) msg->key.data);
	}
	
	free(msg);
}
