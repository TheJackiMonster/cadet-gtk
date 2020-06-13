//
// Created by thejackimonster on 03.06.20.
//

#include "files.h"

#include <fcntl.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/random.h>
#include <sys/stat.h>
#include <sys/sendfile.h>

static void CGTK_make_storage_directory(const char* subdir) {
	const char* path = CGTK_storage_file_path(subdir, "\0");
	
	struct stat stats;
	
	if ((stat(path, &stats) != 0) || (!S_ISDIR(stats.st_mode))) {
		mkdir(path, S_IRWXU);
	}
}

void CGTK_init_storage_directories() {
	CGTK_make_storage_directory("\0");
	CGTK_make_storage_directory(CGTK_STORAGE_UPLOAD_DIR);
	CGTK_make_storage_directory(CGTK_STORAGE_ARCHIVE_DIR);
	CGTK_make_storage_directory(CGTK_STORAGE_CONTACTS_DIR);
	CGTK_make_storage_directory(CGTK_STORAGE_KEYS_DIR);
}

const char* CGTK_storage_file_path(const char* subdir, const char* filename) {
	const struct passwd* pw = getpwuid(getuid());
	const char* home = pw->pw_dir;
	
	const size_t home_len = strlen(home);
	const size_t subdir_len = strlen(subdir);
	const size_t filename_len = strlen(filename);
	
	static const size_t storage_path_len = strlen(CGTK_STORAGE_PATH);
	static char path [PATH_MAX];
	
	size_t offset = 0, remaining = PATH_MAX;
	size_t i;
	
	for (i = 0; i < home_len; i++) {
		path[offset++] = home[i];
	}
	
	remaining -= home_len;
	
	strncpy(path + offset, CGTK_STORAGE_PATH, remaining);
	remaining = (remaining - storage_path_len > 0? remaining - storage_path_len : 0);
	offset += storage_path_len;
	
	strncpy(path + offset, subdir, remaining);
	remaining = (remaining - subdir_len > 0? remaining - subdir_len : 0);
	offset += subdir_len;
	
	strncpy(path + offset, filename, remaining);
	
	path[PATH_MAX - 1] = '\0';
	
	return path;
}

const char* CGTK_generate_random_filename() {
	char random_buffer [CGTK_RANDOM_FILE_BUFFER_SIZE];
	size_t offset = 0;
	
	while (offset < CGTK_RANDOM_FILE_BUFFER_SIZE) {
		ssize_t buffer_read = getrandom(random_buffer + offset, CGTK_RANDOM_FILE_BUFFER_SIZE - offset, 0);
		
		if (buffer_read <= 0) {
			break;
		}
		
		offset += buffer_read;
	}
	
	offset *= 2;
	
	if (offset > CGTK_FILENAME_SIZE) {
		offset = CGTK_FILENAME_SIZE;
	}
	
	static char filename [CGTK_FILENAME_SIZE + 1];
	
	for (size_t i = 0; i < offset; i++) {
		const char value = random_buffer[i / 2];
		
		u_int8_t digit = ((value >> ((i & 1) << 2)) & 0xF);
		
		if (digit < 10) {
			filename[i] = ('0' + digit);
		} else {
			filename[i] = ('A' + digit - 10);
		}
	}
	
	filename[offset] = '\0';
	
	return filename;
}

const char* CGTK_hash_filename(const struct GNUNET_HashCode* hashcode) {
	size_t offset = sizeof(hashcode) * 2;
	
	static char filename [CGTK_FILENAME_SIZE + 1];
	
	for (size_t i = 0; i < offset; i++) {
		const char value = ((char*) hashcode->bits)[i / 2];
		
		u_int8_t digit = ((value >> ((i & 1) << 2)) & 0xF);
		
		if (digit < 10) {
			filename[i] = ('0' + digit);
		} else {
			filename[i] = ('A' + digit - 10);
		}
	}
	
	filename[offset] = '\0';
	
	return filename;
}

char* CGTK_force_suffix_to_filename(char* filename, const char* suffix) {
	const size_t orig_len = strlen(filename);
	const size_t suffix_len = strlen(suffix);
	
	size_t start = orig_len;
	
	if (start > CGTK_FILENAME_SIZE - suffix_len) {
		start = CGTK_FILENAME_SIZE - suffix_len;
	}
	
	for (size_t i = 0; i < suffix_len; i++) {
		filename[start + i] = suffix[i];
	}
	
	filename[start + suffix_len] = '\0';
	
	return filename;
}

int CGTK_check_existence(const char* path) {
	return (access(path, F_OK) == 0);
}

int CGTK_check_directory(const char* path) {
	struct stat stats;
	
	if (stat(path, &stats) != 0) return 0;
	
	return S_ISDIR(stats.st_mode);
}

const char* CGTK_get_filename(const char* path) {
	const char* sep = rindex(path, '/');
	
	if (sep) {
		return (sep + 1);
	} else {
		return path;
	}
}

const char* CGTK_get_extension(const char* path) {
	const char* dot = rindex(path, '.');
	const char* sep = rindex(path, '/');
	
	if ((sep) && (dot > sep)) {
		return dot;
	} else {
		return "\0";
	}
}

const char* CGTK_get_filehash(const char* path) {
	int fd = open(path, O_RDONLY, 0);
	
	if (!fd) {
		return NULL;
	}
	
	struct stat stats;
	if (fstat(fd, &stats) != 0) {
		close(fd);
		return NULL;
	}
	
	const size_t size = stats.st_size;
	
	void* block = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
	
	if (block == MAP_FAILED) {
		close(fd);
		return NULL;
	}
	
	struct GNUNET_HashCode hashcode;
	GNUNET_CRYPTO_hash(block, size, &hashcode);
	
	munmap(block, size);
	close(fd);
	
	return GNUNET_h2s_full(&hashcode);
}

const char* CGTK_upload_via_storage(const char* local_path, const char* extension) {
	const char* filename = CGTK_generate_random_filename();
	static char upload_path [PATH_MAX];
	
	if (extension) {
		strncpy(upload_path, filename, CGTK_FILENAME_SIZE);
		
		filename = CGTK_force_suffix_to_filename(upload_path, extension);
	}
	
	const char* path = CGTK_storage_file_path(CGTK_STORAGE_UPLOAD_DIR, filename);
	strncpy(upload_path, path, PATH_MAX);
	
	if (local_path) {
		int fd_local = open(local_path, O_RDONLY, 0);
		
		if (!fd_local) {
			return NULL;
		}
		
		struct stat stats;
		if (fstat(fd_local, &stats) != 0) {
			close(fd_local);
			return NULL;
		}
		
		int fd_upload = open(upload_path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
		
		if (!fd_upload) {
			close(fd_local);
			return NULL;
		}
		
		sendfile(fd_upload, fd_local, NULL, stats.st_size);
		
		close(fd_local);
		close(fd_upload);
	}
	
	return upload_path;
}

const char* CGTK_access_via_storage(const char* storage_path) {
	const char* path = CGTK_storage_file_path("/\0", CGTK_get_filename(storage_path));
	
	static char access_path [PATH_MAX];
	strncpy(access_path, path, PATH_MAX);
	
	if (!CGTK_check_existence(access_path)) {
		int fd_storage = open(storage_path, O_RDONLY, 0);
		
		if (!fd_storage) {
			return NULL;
		}
		
		struct stat stats;
		if (fstat(fd_storage, &stats) != 0) {
			close(fd_storage);
			return NULL;
		}
		
		int fd = open(access_path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
		
		if (!fd) {
			close(fd_storage);
			return NULL;
		}
		
		sendfile(fd, fd_storage, NULL, stats.st_size);
		
		close(fd_storage);
		close(fd);
		
		cgtk_1tu_key_t key;
		if (CGTK_load_key_for(access_path, &key) != 0) {
			return NULL;
		}
		
		if (CGTK_decrypt_in_storage(access_path, &key) != 0) {
			CGTK_wipe_key(&key);
			return NULL;
		} else {
			CGTK_wipe_key(&key);
		}
	}
	
	return access_path;
}
