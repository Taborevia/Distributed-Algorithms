#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/random.h>
#include <sys/time.h>

#include "rpc_protocol.h"
#include "client.h"

extern ssize_t marshal_request(const rpc_request_t *req, uint8_t *buffer, size_t buffer_size);
extern int unmarshal_response(uint8_t opcode, const uint8_t *buffer, int64_t buffer_size, rpc_response_t *resp);

static int rpc_sockfd = -1;
static struct sockaddr_in server_addr;
static uint64_t current_auth_token = 0;
static uint64_t global_seq_number = 1;

void rpc_init(const char *server_ip, int port) {
    rpc_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(rpc_sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    getrandom(&current_auth_token, sizeof(current_auth_token), 0);
}

int rpc_call(rpc_request_t *req, rpc_response_t *resp) {
    uint8_t send_buf[8192];
    uint8_t recv_buf[8192];

    req->auth_token = current_auth_token;
    req->seq_number = global_seq_number++;

    ssize_t send_len = marshal_request(req, send_buf, sizeof(send_buf));
    if (send_len < 0) return -1;

    int max_retries = 3;
    while (max_retries > 0) {
        sendto(rpc_sockfd, send_buf, send_len, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

        socklen_t srv_len = sizeof(server_addr);
        ssize_t n = recvfrom(rpc_sockfd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&server_addr, &srv_len);

        if (n > 0) {
            unmarshal_response(req->opcode, recv_buf, n, resp);
            if (resp->seq_number == req->seq_number) {
                return 0;
            }
        } else {
            printf("[KLIENT] Timeout! Ponawiam żądanie (pozostało prób: %d)\n", max_retries - 1);
            max_retries--;
        }
    }
    return -1;
}

File *rpc_open(const char *pathname, char *mode) {
    rpc_request_t req = {0};
    rpc_response_t resp = {0};

    req.opcode = RPC_OPEN;
    strncpy(req.args.open_args.pathname, pathname, MAX_PATH_LEN - 1);
    strncpy(req.args.open_args.mode, mode, MAX_MODE_LEN - 1);

    if (rpc_call(&req, &resp) < 0 || resp.status < 0) {
        return NULL;
    }

    File *f = malloc(sizeof(File));
    f->fd = (int)resp.return_value;
    f->auth_token = current_auth_token;
    return f;
}

ssize_t rpc_write(void *buf, size_t count, File *f) {
    if (!f || !buf) return -1;

    rpc_request_t req = {0};
    rpc_response_t resp = {0};

    req.opcode = RPC_WRITE;
    req.args.w_args.fd = f->fd;
    
    uint32_t bytes_to_send = count < MAX_CHUNK_SIZE ? count : MAX_CHUNK_SIZE;
    req.args.w_args.count = bytes_to_send;
    memcpy(req.args.w_args.buf, buf, bytes_to_send);

    if (rpc_call(&req, &resp) < 0) {
        return -1;
    }

    if (resp.status < 0) {
        return resp.status;
    }

    return resp.return_value;
}

off_t rpc_lseek(File *f, off_t offset, int whence) {
    if (!f) return -1;

    rpc_request_t req = {0};
    rpc_response_t resp = {0};

    req.opcode = RPC_LSEEK;
    req.args.lseek_args.fd = f->fd;
    req.args.lseek_args.offset = offset;
    req.args.lseek_args.whence = whence;

    if (rpc_call(&req, &resp) < 0) {
        return -1;
    }
    
    if (resp.status < 0) {
        return resp.status;
    }

    return (off_t)resp.return_value;
}

size_t rpc_read(void *buf, size_t count, File *f) {
    if (!f) return -1;
    
    rpc_request_t req = {0};
    rpc_response_t resp = {0};

    req.opcode = RPC_READ;
    req.args.r_args.fd = f->fd;
    
    req.args.r_args.count = count;

    if (rpc_call(&req, &resp) < 0) {
        return -1;
    }
    
    if (resp.status < 0) {
        return resp.status;
    }

    memcpy(buf, resp.data, resp.return_value);

    return resp.return_value;
}

int rpc_close(File *f) {
    if (!f) return -1;

    rpc_request_t req = {0};
    rpc_response_t resp = {0};

    req.opcode = RPC_CLOSE;
    req.args.close_args.fd = f->fd;

    if (rpc_call(&req, &resp) < 0) {
        return -1;
    }

    free(f);

    return resp.status;
}

int rpc_chmod(const char *pathname, mode_t mode) {
    rpc_request_t req = {0};
    rpc_response_t resp = {0};

    req.opcode = RPC_CHMOD;
    strncpy(req.args.chmod_args.pathname, pathname, MAX_PATH_LEN);
    req.args.chmod_args.mode = (uint32_t)mode;

    if (rpc_call(&req, &resp) < 0) return -1;
    return resp.status; 
}

int rpc_unlink(const char *pathname) {
    rpc_request_t req = {0};
    rpc_response_t resp = {0};

    req.opcode = RPC_UNLINK;
    strncpy(req.args.unlink_args.pathname, pathname, MAX_PATH_LEN);

    if (rpc_call(&req, &resp) < 0) return -1;
    return resp.status;
}

int rpc_rename(const char *oldpath, const char *newpath) {
    rpc_request_t req = {0};
    rpc_response_t resp = {0};

    req.opcode = RPC_RENAME;
    strncpy(req.args.rename_args.oldpath, oldpath, MAX_PATH_LEN);
    strncpy(req.args.rename_args.newpath, newpath, MAX_PATH_LEN);

    if (rpc_call(&req, &resp) < 0) return -1;
    return resp.status;
}