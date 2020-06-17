//
// Created by thejackimonster on 03.06.20.
//

#ifndef CADET_GTK_STORAGE_KEYS_H
#define CADET_GTK_STORAGE_KEYS_H

#include <gnunet/gnunet_config.h>
#include <gnunet/gnunet_crypto_lib.h>

#define CGTK_1TU_KEY_SIZE (sizeof(struct GNUNET_CRYPTO_SymmetricSessionKey))

typedef union {
	struct GNUNET_CRYPTO_SymmetricSessionKey key;
	char key_data [CGTK_1TU_KEY_SIZE];
} cgtk_1tu_key_t;

void CGTK_generate_new_key(cgtk_1tu_key_t* key);

void CGTK_wipe_key(cgtk_1tu_key_t* key);

int CGTK_encrypt_in_storage(const char* path, const cgtk_1tu_key_t* key);

int CGTK_decrypt_in_storage(const char* path, const cgtk_1tu_key_t* key);

int CGTK_hash_compare_in_storage(const char* path, const cgtk_1tu_key_t* key, const char* hash);

int CGTK_store_key_for(const char* path, const cgtk_1tu_key_t* key);

int CGTK_load_key_for(const char* path, cgtk_1tu_key_t* key);

const char* CGTK_key_to_string(const cgtk_1tu_key_t* key);

int CGTK_key_from_string(const char* string, cgtk_1tu_key_t* key);

#endif //CADET_GTK_STORAGE_KEYS_H
