//
// Created by thejackimonster on 30.04.20.
//

#ifndef CADET_GTK_MSG_H
#define CADET_GTK_MSG_H

#include <stdint.h>
#include <jansson.h>

#define MSG_DEC_KIND_BIT 0x01u
#define MSG_DEC_TIMESTAMP_BIT 0x02u
#define MSG_DEC_SENDER_BIT 0x04u
#define MSG_DEC_CONTENT_BIT 0x08u
#define MSG_DEC_WHO_BIT 0x10u
#define MSG_DEC_PARTICIPANTS_BIT 0x20u
#define MSG_DEC_

#define MSG_DEC_COMPLETE_BITS 0x3Fu

typedef enum {
	MSG_KIND_TALK = 1,
	MSG_KIND_JOIN = 2,
	MSG_KIND_LEAVE = 3,
	MSG_KIND_INFO = 4,
	MSG_KIND_FILE = 5,
	
	MSG_KIND_UNKNOWN = 0
} msg_kind_t;

typedef struct msg_t {
	msg_kind_t kind;
	time_t timestamp;
	
	const char* sender;
	const char* content;
	
	const char* who;
	
	const char** participants;
	
	u_int8_t local;
	uint32_t decoding;
} msg_t;

const char* CGTK_encode_message(const msg_t* msg, size_t* message_len);

msg_t* CGTK_decode_message(const char* message, size_t message_len);

void CGTK_repair_message(msg_t* msg, const char* message, const char* sender);

void CGTK_free_message(msg_t* msg);

#endif //CADET_GTK_MSG_H
