//
// Created by thejackimonster on 30.04.20.
//

#ifndef CADET_GTK_JSON_H
#define CADET_GTK_JSON_H

#include <jansson.h>

typedef unsigned char bool_t;
typedef unsigned long msg_decoding_t;

#define MSG_DEC_KIND_BIT 0x01L
#define MSG_DEC_TIMESTAMP_BIT 0x02L
#define MSG_DEC_SENDER_BIT 0x04L
#define MSG_DEC_CONTENT_BIT 0x08L
#define MSG_DEC_WHO_BIT 0x10L
#define MSG_DEC_PARTICIPANTS_BIT 0x20L

#define MSG_DEC_COMPLETE_BITS 0x3FL

typedef enum {
	MSG_KIND_TALK = 1,
	MSG_KIND_JOIN = 2,
	MSG_KIND_LEAVE = 3,
	MSG_KIND_INFO = 4,
	
	MSG_KIND_UNKNOWN = 0
} msg_kind_t;

typedef struct msg_t {
	msg_kind_t kind;
	time_t timestamp;
	
	const char* sender;
	const char* content;
	
	const char* who;
	
	const char** participants;
	
	bool_t local;
	msg_decoding_t decoding;
} msg_t;

const char* CGTK_encode_message(const msg_t* msg, size_t* message_len);

msg_t* CGTK_decode_message(const char* message);

void CGTK_free_message(msg_t* msg);

#endif //CADET_GTK_JSON_H
