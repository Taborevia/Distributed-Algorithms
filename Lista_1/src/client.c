#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/random.h>
#include <sys/time.h>

#include "rpc_protocol.h"

extern ssize_t marshal_request(const rpc_request_t *req, uint8_t *buffer, size_t buffer_size);

// Pomocnicza funkcja do rozpakowania odpowiedzi (analogiczna do marshalingu)
extern int unmarshal_response(const uint8_t *buffer, size_t buffer_len, rpc_response_t *resp);

// Globalny stan klienta
static int rpc_sockfd = -1;
static struct sockaddr_in server_addr;
static uint64_t current_auth_token = 0;
static uint64_t global_seq_number = 1;

// Inicjalizacja połączenia
void rpc_init(const char *server_ip, int port) {
    rpc_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    // Ustawienie timeoutu dla recvfrom na 1 sekundę
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(rpc_sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // Generowanie tokena uwierzytelniającego (zgodnie z poleceniem)
    getrandom(&current_auth_token, sizeof(current_auth_token), 0);
}

// Główna funkcja wysyłająca z logiką RETRY
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
            unmarshal_response(recv_buf, n, resp);
            if (resp->seq_number == req->seq_number) {
                return 0; // Sukces
            }
            // Zły numer sekwencyjny? Czekamy dalej (w pętli, stary pakiet)
        } else {
            // Timeout - zmniejszamy licznik i powtarzamy
            printf("[KLIENT] Timeout! Ponawiam żądanie (pozostało prób: %d)\n", max_retries - 1);
            max_retries--;
        }
    }
    return -1; // Błąd po wszystkich próbach (Fail)
}

/* --- API DLA UŻYTKOWNIKA --- */

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
    req.args.rw_args.fd = f->fd;
    
    // Obsługa Opcji 1: wysyłamy maksymalnie MAX_CHUNK_SIZE
    uint32_t bytes_to_send = count < MAX_CHUNK_SIZE ? count : MAX_CHUNK_SIZE;
    req.args.rw_args.count = bytes_to_send;
    memcpy(req.args.rw_args.buf, buf, bytes_to_send);

    if (rpc_call(&req, &resp) < 0) {
        return -1; // Awaria komunikacji
    }

    if (resp.status < 0) {
        return resp.status; // Błąd operacji na pliku
    }

    return resp.return_value; // Zwracamy ile bajtów zapisał serwer
}

size_t rpc_read(void *buf, size_t count, File *f) {
    if (!f) return -1;
    
    rpc_request_t req = {0};
    rpc_response_t resp = {0};

    req.opcode = RPC_READ;
    req.args.rw_args.fd = f->fd;
    
    // Obsługa Opcji 1: wysyłamy maksymalnie MAX_CHUNK_SIZE
    uint32_t bytes_to_send = count < MAX_CHUNK_SIZE ? count : MAX_CHUNK_SIZE;
    req.args.rw_args.count = bytes_to_send;
    // memcpy(req.args.rw_args.buf, buf, bytes_to_send);
    printf("test2\n");

    if (rpc_call(&req, &resp) < 0) {
        return -1; // Awaria komunikacji
    }
    printf("test1\n");

    if (resp.status < 0) {
        return resp.status; // Błąd operacji na pliku
    }

    return resp.return_value; // Zwracamy ile bajtów zapisał serwer
}