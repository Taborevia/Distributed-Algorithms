#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <endian.h>
#include <errno.h>

#include "rpc_protocol.h"

extern int unmarshal_request(const uint8_t *buffer, size_t buffer_size, rpc_request_t *req);
extern ssize_t marshal_response(uint8_t opcode, const rpc_response_t *resp, uint8_t *buffer, int64_t buffer_size);

#define PORT 8080

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    uint8_t recv_buffer[8192];
    uint8_t send_buffer[8192];

    // CACHE
    uint64_t last_seq_number = 0;
    rpc_response_t last_response;
    memset(&last_response, 0, sizeof(last_response));

    // SOCKET SETUP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("error creating socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("error binding socket");
        exit(EXIT_FAILURE);
    }

    printf("[SERWER] listening on port %d...\n", PORT);

    while (1) {
        ssize_t n = recvfrom(sockfd, recv_buffer, sizeof(recv_buffer), 0, 
                             (struct sockaddr *)&client_addr, &client_len);
        if (n < 0) continue;

        rpc_request_t req;
        if (unmarshal_request(recv_buffer, n, &req) < 0) {
            fprintf(stderr, "[SERWER] wrong package.\n");
            continue;
        }

        printf("[SERWER] Got task: Opcode=%d, Seq=%lu\n", req.opcode, req.seq_number);

        // --- CACHE ---
        if (req.seq_number == last_seq_number) {
            printf("[SERWER] Duplicate! Return value from cache.\n");
            ssize_t resp_len = marshal_response(req.opcode, &last_response, send_buffer, sizeof(send_buffer));
            sendto(sockfd, send_buffer, resp_len, 0, (struct sockaddr *)&client_addr, client_len);
            continue;
        } else if (req.seq_number < last_seq_number) {
            printf("[SERWER] Reject old package (Seq=%lu).\n", req.seq_number);
            continue;
        }

        rpc_response_t resp;
        resp.seq_number = req.seq_number;
        resp.status = 0;
        resp.return_value = 0;

        switch (req.opcode) {
            case RPC_OPEN: {
                // To simplify uses O_RDWR
                int flags = O_RDWR | O_CREAT; 
                int fd = open(req.args.open_args.pathname, flags, 0644);
                if (fd < 0) {
                    resp.status = -errno;
                } else {
                    resp.return_value = fd;
                    printf("[SERWER] Opened file %s, local fd=%d\n", req.args.open_args.pathname, fd);
                }
                break;
            }
            case RPC_WRITE: {
                ssize_t written = write(req.args.w_args.fd, req.args.w_args.buf, req.args.w_args.count);
                if (written < 0) {
                    resp.status = -errno;
                } else {
                    resp.return_value = written;
                    printf("[SERWER] Written %ld bytes to fd=%d\n", written, req.args.w_args.fd);
                }
                break;
            }
            case RPC_LSEEK: {
                off_t new_offset = lseek(req.args.lseek_args.fd, 
                                         req.args.lseek_args.offset, 
                                         req.args.lseek_args.whence);
                if (new_offset == (off_t)-1) {
                    resp.status = -errno;
                } else {
                    resp.return_value = new_offset;
                    printf("[SERWER] Lseek on fd=%d, new offset=%ld\n", 
                           req.args.lseek_args.fd, new_offset);
                }
                break;
            }
            case RPC_READ: {
                printf("[SERWER] Try to read %u bytes from fd=%d\n", req.args.r_args.count, req.args.r_args.fd);
                ssize_t read_bytes = read(req.args.r_args.fd, resp.data, req.args.r_args.count);
                if (read_bytes < 0) {
                    printf("ERROR\n");
                    resp.status = -errno;
                } else {
                    resp.return_value = read_bytes;
                    printf("[SERWER] Read %ld bytes from fd=%d\n", read_bytes, req.args.r_args.fd);
                }
                break;
            }
            case RPC_CLOSE: {
                if (close(req.args.close_args.fd) < 0) {
                    resp.status = -errno;
                } else {
                    printf("[SERWER] Closed fd=%d\n", req.args.close_args.fd);
                }
                break;
            }
            case RPC_CHMOD: {
                int res = chmod(req.args.chmod_args.pathname, (mode_t)req.args.chmod_args.mode);
                resp.status = (res < 0) ? -errno : 0;
                printf("[SERWER] chmod %s to %o: status %d\n", req.args.chmod_args.pathname, req.args.chmod_args.mode, res);
                break;
            }
            case RPC_UNLINK: {
                int res = unlink(req.args.unlink_args.pathname);
                resp.status = (res < 0) ? -errno : 0;
                printf("[SERWER] unlink %s: status %d\n", req.args.unlink_args.pathname, res);
                break;
            }
            case RPC_RENAME: {
                int res = rename(req.args.rename_args.oldpath, req.args.rename_args.newpath);
                resp.status = (res < 0) ? -errno : 0;
                printf("[SERWER] rename %s -> %s: status %d\n", req.args.rename_args.oldpath, req.args.rename_args.newpath, res);
                break;
            }
            default:
                resp.status = -ENOSYS;
                break;
        }

        // save to cache
        last_seq_number = req.seq_number;
        last_response = resp;

        // return result
        ssize_t resp_len = marshal_response(req.opcode, &resp, send_buffer, sizeof(send_buffer));
        sendto(sockfd, send_buffer, resp_len, 0, (struct sockaddr *)&client_addr, client_len);
    }

    close(sockfd);
    return 0;
}