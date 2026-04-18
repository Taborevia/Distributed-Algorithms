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

// Deklaracje funkcji marshalingu (z poprzedniego kroku)
extern int unmarshal_request(const uint8_t *buffer, size_t buffer_len, rpc_request_t *req);

// Pomocnicza funkcja do pakowania odpowiedzi (żeby zachować Big-Endian)
extern ssize_t marshal_response(const rpc_response_t *resp, uint8_t *buffer, size_t buffer_size);

#define PORT 8080

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    uint8_t recv_buffer[8192];
    uint8_t send_buffer[8192];

    // INICJALIZACJA CACHE (Uwaga 4 z zadania)
    uint64_t last_seq_number = 0;
    rpc_response_t last_response;
    memset(&last_response, 0, sizeof(last_response));

    // 1. Tworzenie gniazda UDP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Błąd tworzenia gniazda");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Błąd bindowania");
        exit(EXIT_FAILURE);
    }

    printf("[SERWER] Nasłuchiwanie na porcie %d...\n", PORT);

    while (1) {
        ssize_t n = recvfrom(sockfd, recv_buffer, sizeof(recv_buffer), 0, 
                             (struct sockaddr *)&client_addr, &client_len);
        if (n < 0) continue;

        rpc_request_t req;
        if (unmarshal_request(recv_buffer, n, &req) < 0) {
            fprintf(stderr, "[SERWER] Otrzymano niepoprawny pakiet.\n");
            continue;
        }

        printf("[SERWER] Odebrano żądanie: Opcode=%d, Seq=%lu\n", req.opcode, req.seq_number);

        // --- OBSŁUGA CACHE I IDEMPOTENTNOŚCI ---
        if (req.seq_number == last_seq_number) {
            printf("[SERWER] Duplikat żądania! Zwracam wynik z pamięci podręcznej.\n");
            ssize_t resp_len = marshal_response(&last_response, send_buffer, sizeof(send_buffer));
            sendto(sockfd, send_buffer, resp_len, 0, (struct sockaddr *)&client_addr, client_len);
            continue;
        } else if (req.seq_number < last_seq_number) {
            printf("[SERWER] Odrzucam przestarzały pakiet (Seq=%lu).\n", req.seq_number);
            continue; // Ignorujemy paczkę widmo
        }

        // --- WYKONANIE WŁAŚCIWEJ OPERACJI POSIX ---
        rpc_response_t resp;
        resp.seq_number = req.seq_number;
        resp.status = 0;
        resp.return_value = 0;

        switch (req.opcode) {
            case RPC_OPEN: {
                // Konwersja prostego trybu (dla uproszczenia zakładamy że klient wysyła "w" lub "r")
                int flags = O_RDWR | O_CREAT; 
                int fd = open(req.args.open_args.pathname, flags, 0644);
                if (fd < 0) {
                    resp.status = -errno;
                } else {
                    resp.return_value = fd;
                    printf("[SERWER] Otwarto plik %s, lokalny fd=%d\n", req.args.open_args.pathname, fd);
                }
                break;
            }
            case RPC_WRITE: {
                ssize_t written = write(req.args.rw_args.fd, req.args.rw_args.buf, req.args.rw_args.count);
                if (written < 0) {
                    resp.status = -errno;
                } else {
                    resp.return_value = written;
                    printf("[SERWER] Zapisano %ld bajtów do fd=%d\n", written, req.args.rw_args.fd);
                }
                break;
            }
            case RPC_READ: {
                printf("[SERWER] Próba odczytu %u bajtów z fd=%d\n", req.args.rw_args.count, req.args.rw_args.fd);
                ssize_t read_bytes = read(req.args.rw_args.fd, req.args.rw_args.buf, req.args.rw_args.count);
                if (read_bytes < 0) {
                    resp.status = -errno;
                } else {
                    resp.return_value = read_bytes;
                    // resp.data = req.args.rw_args.buf;
                    printf("[SERWER] Odczytano %ld bajtów z fd=%d\n", read_bytes, req.args.rw_args.fd);
                }
            }
            default:
                resp.status = -ENOSYS; // Niezaimplementowane
                break;
        }

        // Zapisz do cache
        last_seq_number = req.seq_number;
        last_response = resp;

        // Odeślij odpowiedź
        ssize_t resp_len = marshal_response(&resp, send_buffer, sizeof(send_buffer));
        sendto(sockfd, send_buffer, resp_len, 0, (struct sockaddr *)&client_addr, client_len);
    }

    close(sockfd);
    return 0;
}