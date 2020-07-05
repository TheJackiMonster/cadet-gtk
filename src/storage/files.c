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
	CGTK_make_storage_directory(CGTK_STORAGE_ARCHIVE_DIR);
	CGTK_make_storage_directory(CGTK_STORAGE_CHATS_DIR);
	CGTK_make_storage_directory(CGTK_STORAGE_CONTACTS_DIR);
	CGTK_make_storage_directory(CGTK_STORAGE_DOWNLOAD_DIR);
	CGTK_make_storage_directory(CGTK_STORAGE_KEYS_DIR);
	CGTK_make_storage_directory(CGTK_STORAGE_UPLOAD_DIR);
}

const char* CGTK_home_file_path(const char* subdir, const char* filename) {
	const struct passwd* pw = getpwuid(getuid());
	const char* home = pw->pw_dir;
	
	const size_t home_len = strlen(home);
	const size_t subdir_len = strlen(subdir);
	
	static char path [CGTK_PATH_SIZE];
	
	size_t offset = 0, remaining = CGTK_PATH_SIZE;
	size_t i;
	
	for (i = 0; i < home_len; i++) {
		path[offset++] = home[i];
	}
	
	remaining -= home_len;
	
	strncpy(path + offset, subdir, remaining);
	remaining = (remaining - subdir_len > 0? remaining - subdir_len : 0);
	offset += subdir_len;
	
	strncpy(path + offset, filename, remaining);
	
	path[CGTK_PATH_SIZE - 1] = '\0';
	
	return path;
}

const char* CGTK_storage_file_path(const char* subdir, const char* filename) {
	const char* storage = CGTK_home_file_path(CGTK_STORAGE_PATH, subdir);
	
	const size_t storage_len = strlen(storage);
	
	static char path [CGTK_PATH_SIZE];
	
	size_t offset = 0, remaining = CGTK_PATH_SIZE;
	size_t i;
	
	for (i = 0; i < storage_len; i++) {
		path[offset++] = storage[i];
	}
	
	remaining -= storage_len;

	strncpy(path + offset, filename, remaining);
	
	path[CGTK_PATH_SIZE - 1] = '\0';
	
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

int CGTK_check_existence(const char* path) {
	return (access(path, F_OK) == 0);
}

int CGTK_check_directory(const char* path) {
	struct stat stats;
	
	if (stat(path, &stats) != 0) return 0;
	
	return S_ISDIR(stats.st_mode);
}

int CGTK_check_storage_subdir(const char* path, const char* subdir) {
	const char* storage = CGTK_home_file_path(CGTK_STORAGE_PATH, subdir);
	
	return (strstr(path, storage) == path);
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
	
	if (dot > sep) {
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

int CGTK_copy_file(const char* src_path, const char* dst_path) {
	if ((!src_path) || (!dst_path)) {
		return -1;
	}
	
	int src = open(src_path, O_RDONLY, 0);
	
	if (!src) {
		return -2;
	}
	
	struct stat stats;
	if (fstat(src, &stats) != 0) {
		close(src);
		return -3;
	}
	
	int dst = open(dst_path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	
	if (!dst) {
		close(src);
		return -4;
	}
	
	const ssize_t copied = sendfile(dst, src, NULL, stats.st_size);
	
	close(src);
	close(dst);
	
	return (copied - stats.st_size);
}

int CGTK_remove_file(const char* path) {
	return remove(path);
}
