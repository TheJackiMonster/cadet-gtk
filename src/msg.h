//
// Created by thejackimonster on 30.04.20.
//

#ifndef CADET_GTK_MSG_H
#define CADET_GTK_MSG_H

#include <stdint.h>
#include <jansson.h>

#define MSG_DEC_KIND_BIT 0x001u
#define MSG_DEC_TIMESTAMP_BIT 0x002u
#define MSG_DEC_SENDER_BIT 0x004u
#define MSG_DEC_CONTENT_BIT 0x008u
#define MSG_DEC_WHO_BIT 0x010u
#define MSG_DEC_PARTICIPANTS_BIT 0x020u
#define MSG_DEC_PUBLISHER_BIT 0x040u
#define MSG_DEC_URI_BIT 0x080u
#define MSG_DEC_HASH_BIT 0x100u
#define MSG_DEC_NAME_BIT 0x200u
#define MSG_DEC_TYPE_BIT 0x400u
#define MSG_DEC_DATA_BIT 0x800u

#define MSG_DEC_COMPLETE_BITS 0xFFFu

typedef enum msg_kind_t {
	MSG_KIND_TALK = 1,
	MSG_KIND_JOIN = 2,
	MSG_KIND_LEAVE = 3,
	MSG_KIND_INFO = 4,
	MSG_KIND_FILE = 5,
	MSG_KIND_KEY = 6,
	
	MSG_KIND_UNKNOWN = 0
} msg_kind_t;

typedef enum msg_key_t {
	MSG_KEY_1TU = 1,
	MSG_KEY_GPG = 2,
	
	MSG_KEY_UNKNOWN = 0
} msg_key_t;

typedef struct msg_t {
	msg_kind_t kind;
	time_t timestamp;
	
	union {
		struct {
			const char* sender;
			const char* content;
		} talk;
		
		struct {
			const char* who;
		} join_leave;
		
		struct {
			const char** participants;
		} info;
		
		struct {
			const char* publisher;
			const char* uri;
			const char* hash;
			const char* name;
			
			const char* path;
		} file;
		
		struct {
			msg_key_t type;
			const char* data;
		} key;
	};
	
	u_int8_t local;
	uint32_t decoding;
} msg_t;

const char* CGTK_encode_message(const msg_t* msg, size_t* message_len);

msg_t* CGTK_decode_message(const char* message, size_t message_len);

void CGTK_repair_message(msg_t* msg, const char* message, size_t message_len, const char* sender);

void CGTK_free_message(msg_t* msg);

#endif //CADET_GTK_MSG_H
