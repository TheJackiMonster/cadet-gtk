//
// Created by thejackimonster on 03.06.20.
//

#include "keys.h"
#include "files.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

void CGTK_generate_new_key(cgtk_1tu_key_t* key) {
	GNUNET_CRYPTO_symmetric_create_session_key(&(key->key));
}

void CGTK_wipe_key(cgtk_1tu_key_t* key) {
	GNUNET_CRYPTO_zero_keys(key, sizeof(cgtk_1tu_key_t));
}

int CGTK_encrypt_in_storage(const char* path, const cgtk_1tu_key_t* key) {
	struct stat stats;
	
	if ((stat(path, &stats) != 0) || (!S_ISREG(stats.st_mode))) {
		return -1;
	}
	
	const size_t size = stats.st_size;
	
	int fd = open(path, O_RDWR);
	
	if (!fd) {
		return -2;
	}
	
	void* block = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	
	if (block == MAP_FAILED) {
		close(fd);
		return -3;
	}
	
	struct GNUNET_CRYPTO_SymmetricInitializationVector iv;
	memset(&iv, 0, sizeof(iv));
	
	ssize_t result = GNUNET_CRYPTO_symmetric_encrypt(block, size, &(key->key), &iv, block);
	
	munmap(block, size);
	close(fd);
	
	return (result == size? 0 : -4);
}

int CGTK_decrypt_in_storage(const char* path, const cgtk_1tu_key_t* key) {
	struct stat stats;
	
	if ((stat(path, &stats) != 0) || (!S_ISREG(stats.st_mode))) {
		return -1;
	}
	
	const size_t size = stats.st_size;
	
	int fd = open(path, O_RDWR);
	
	if (!fd) {
		return -2;
	}
	
	void* block = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	
	if (block == MAP_FAILED) {
		close(fd);
		return -3;
	}
	
	struct GNUNET_CRYPTO_SymmetricInitializationVector iv;
	memset(&iv, 0, sizeof(iv));
	
	ssize_t result = GNUNET_CRYPTO_symmetric_decrypt(block, size, &(key->key), &iv, block);
	
	munmap(block, size);
	close(fd);
	
	return (result == size? 0 : -4);
}

int CGTK_hash_compare_in_storage(const char* path, const cgtk_1tu_key_t* key, const char* hash) {
	struct stat stats;
	
	if ((stat(path, &stats) != 0) || (!S_ISREG(stats.st_mode))) {
		return -1;
	}
	
	const size_t size = stats.st_size;
	
	int fd = open(path, O_RDWR);
	
	if (!fd) {
		return -2;
	}
	
	void* block = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	
	if (block == MAP_FAILED) {
		close(fd);
		return -3;
	}
	
	struct GNUNET_CRYPTO_SymmetricInitializationVector iv;
	memset(&iv, 0, sizeof(iv));
	
	ssize_t result = GNUNET_CRYPTO_symmetric_decrypt(block, size, &(key->key), &iv, block);
	
	int cmp_result = -5;
	
	if (result == size) {
		struct GNUNET_HashCode hashcode;
		GNUNET_CRYPTO_hash(block, size, &hashcode);
		
		const char* hash_string = GNUNET_h2s_full(&hashcode);
		
		cmp_result = strcmp(hash, hash_string);
	}
	
	memset(&iv, 0, sizeof(iv));
	
	result = GNUNET_CRYPTO_symmetric_encrypt(block, result, &(key->key), &iv, block);
	
	munmap(block, size);
	close(fd);
	
	return (result == size? cmp_result : -4);
}

int CGTK_store_key_for(const char* path, const cgtk_1tu_key_t* key) {
	const char* filename = CGTK_get_filename(path);
	const char* key_path = CGTK_storage_file_path(CGTK_STORAGE_KEYS_DIR, filename);
	
	int fd = open(key_path, O_WRONLY | O_CREAT, S_IRUSR);
	
	if (!fd) {
		return -1;
	}
	
	ssize_t result = write(fd, key->key_data, sizeof(cgtk_1tu_key_t));
	close(fd);
	
	return (result == sizeof(cgtk_1tu_key_t)? 0 : -2);
}

int CGTK_load_key_for(const char* path, cgtk_1tu_key_t* key) {
	const char* filename = CGTK_get_filename(path);
	const char* key_path = CGTK_storage_file_path(CGTK_STORAGE_KEYS_DIR, filename);
	
	int fd = open(key_path, O_RDONLY);
	
	if (!fd) {
		return -1;
	}
	
	ssize_t result = read(fd, key->key_data, sizeof(cgtk_1tu_key_t));
	close(fd);
	
	return (result == sizeof(cgtk_1tu_key_t)? 0 : -2);
}

#define CGTK_1TU_KEY_STRING_SIZE (CGTK_1TU_KEY_SIZE * 2)

const char* CGTK_key_to_string(const cgtk_1tu_key_t* key) {
	static char string [CGTK_1TU_KEY_STRING_SIZE + 1];
	
	for (size_t i = 0; i < CGTK_1TU_KEY_STRING_SIZE; i++) {
		const char value = key->key_data[i / 2];
		
		u_int8_t digit = ((value >> ((i & 1) << 2)) & 0xF);
		
		if (digit < 10) {
			string[i] = ('0' + digit);
		} else {
			string[i] = ('A' + digit - 10);
		}
	}
	
	string[CGTK_1TU_KEY_STRING_SIZE] = '\0';
	return string;
}

static uint8_t CGTK_digit_from_char(char ch) {
	if (ch >= 'A') {
		return (10 + ch - 'A');
	} else {
		return (ch - '0');
	}
}

int CGTK_key_from_string(const char* string, cgtk_1tu_key_t* key) {
	if (strlen(string) != CGTK_1TU_KEY_STRING_SIZE) {
		return -1;
	}
	
	for (size_t i = 0; i < CGTK_1TU_KEY_SIZE; i++) {
		const char value = (
				(CGTK_digit_from_char(string[i * 2])) |
				(CGTK_digit_from_char(string[i * 2]) << 4)
		);
		
		key->key_data[i] = value;
	}
	
	return 0;
}
