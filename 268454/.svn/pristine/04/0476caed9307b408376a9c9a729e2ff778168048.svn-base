#ifndef CLIENT_H
#define CLIENT_H

#include <sys/types.h>
#include <stdint.h>
#include "rpc_protocol.h"

typedef struct {
    int fd;
    uint64_t auth_token;
} File;


void rpc_init(const char *server_ip, int port);

File* rpc_open(const char *pathname, char *mode);
ssize_t rpc_write(void *buf, size_t count, File *f);
size_t rpc_read(void *buf, size_t count, File *f);
off_t rpc_lseek(File *f, off_t offset, int whence);
int rpc_close(File *f);

int rpc_chmod(const char *pathname, mode_t mode);
int rpc_unlink(const char *pathname);
int rpc_rename(const char *oldpath, const char *newpath);

#endif