#include <stdint.h>
#include <string.h>
#include <endian.h>
#include "rpc_protocol.h"

/* --- MARSHALING (KLIENT: Struktura -> Bufor UDP) --- */
// Zwraca liczbę zapisanych bajtów lub -1 w przypadku błędu
ssize_t marshal_request(const rpc_request_t *req, uint8_t *buffer, size_t buffer_size) {
    if (!req || !buffer || buffer_size < sizeof(rpc_request_t)) {
        return -1; 
    }

    uint8_t *ptr = buffer; // Wskaźnik, którym będziemy przesuwać się po buforze

    // 1. Zapisujemy opcode (1 bajt, nie wymaga zmiany Endianness)
    *ptr = req->opcode;
    ptr += sizeof(uint8_t);

    // 2. Zapisujemy auth_token (8 bajtów, konwersja do Big-Endian)
    uint64_t net_auth = htobe64(req->auth_token);
    memcpy(ptr, &net_auth, sizeof(uint64_t));
    ptr += sizeof(uint64_t);

    // 3. Zapisujemy seq_number (8 bajtów, konwersja do Big-Endian)
    uint64_t net_seq = htobe64(req->seq_number);
    memcpy(ptr, &net_seq, sizeof(uint64_t));
    ptr += sizeof(uint64_t);

    // 4. Zapisujemy odpowiednie argumenty w zależności od operacji
    switch (req->opcode) {
        case RPC_OPEN:
            // Ciągi znaków kopiujemy bezpośrednio (są to tablice bajtów)
            memcpy(ptr, req->args.open_args.pathname, MAX_PATH_LEN);
            ptr += MAX_PATH_LEN;
            memcpy(ptr, req->args.open_args.mode, MAX_MODE_LEN);
            ptr += MAX_MODE_LEN;
            break;

        case RPC_WRITE: {
            // Deskryptor fd (32-bity)
            uint32_t net_fd = htobe32((uint32_t)req->args.w_args.fd);
            memcpy(ptr, &net_fd, sizeof(uint32_t));
            ptr += sizeof(uint32_t);

            // Zmienna count (32-bity)
            uint32_t net_count = htobe32(req->args.w_args.count);
            memcpy(ptr, &net_count, sizeof(uint32_t));
            ptr += sizeof(uint32_t);

            // Bufor danych - kopiujemy tylko tyle bajtów, ile wskazuje count
            memcpy(ptr, req->args.w_args.buf, req->args.w_args.count);
            ptr += req->args.w_args.count;
            break;
        }

        case RPC_READ: {
            // Deskryptor fd (32-bity)
            uint32_t net_fd = htobe32((uint32_t)req->args.w_args.fd);
            memcpy(ptr, &net_fd, sizeof(uint32_t));
            ptr += sizeof(uint32_t);

            // Zmienna count (32-bity)
            uint32_t net_count = htobe32(req->args.w_args.count);
            memcpy(ptr, &net_count, sizeof(uint32_t));
            ptr += sizeof(uint32_t);

            break;
        }

        // TUTAJ dodaj analogiczne bloki (case) dla RPC_READ, RPC_LSEEK, itd.
        // Zawsze pamiętaj o htobe32 dla intów i htobe64 dla long/off_t.

        default:
            // Jeśli opcode nie wymaga dodatkowych argumentów lub jest nieznany
            break;
    }

    // Obliczamy całkowity rozmiar paczki do wysłania
    return (ssize_t)(ptr - buffer);
}


/* --- DEMARSHALING (SERWER: Bufor UDP -> Struktura) --- */
// Zwraca 0 w przypadku sukcesu lub -1 w przypadku błędu
int unmarshal_request(const uint8_t *buffer, size_t buffer_len, rpc_request_t *req) {
    if (!buffer || !req || buffer_len < 17) {
        // 17 = 1 bajt opcode + 8 bajtów token + 8 bajtów seq_number (minimum)
        return -1;
    }

    const uint8_t *ptr = buffer;

    // 1. Odczytujemy opcode
    req->opcode = *ptr;
    ptr += sizeof(uint8_t);

    // 2. Odczytujemy auth_token (konwersja z Big-Endian na format hosta)
    uint64_t net_auth;
    memcpy(&net_auth, ptr, sizeof(uint64_t));
    req->auth_token = be64toh(net_auth);
    ptr += sizeof(uint64_t);

    // 3. Odczytujemy seq_number
    uint64_t net_seq;
    memcpy(&net_seq, ptr, sizeof(uint64_t));
    req->seq_number = be64toh(net_seq);
    ptr += sizeof(uint64_t);

    // 4. Odczytujemy argumenty w zależności od operacji
    switch (req->opcode) {
        case RPC_OPEN:
            // Upewniamy się, że w buforze jest wystarczająco danych
            if (buffer_len - (ptr - buffer) < (MAX_PATH_LEN + MAX_MODE_LEN)) return -1;
            
            memcpy(req->args.open_args.pathname, ptr, MAX_PATH_LEN);
            ptr += MAX_PATH_LEN;
            memcpy(req->args.open_args.mode, ptr, MAX_MODE_LEN);
            ptr += MAX_MODE_LEN;
            break;

        case RPC_WRITE: {
            if (buffer_len - (ptr - buffer) < (sizeof(uint32_t) * 2)) return -1;

            uint32_t net_fd, net_count;
            
            memcpy(&net_fd, ptr, sizeof(uint32_t));
            req->args.w_args.fd = (int)be32toh(net_fd);
            ptr += sizeof(uint32_t);

            memcpy(&net_count, ptr, sizeof(uint32_t));
            req->args.w_args.count = be32toh(net_count);
            ptr += sizeof(uint32_t);

            // Zabezpieczenie przed przepełnieniem bufora
            if (req->args.w_args.count > MAX_CHUNK_SIZE) return -1;
            if (buffer_len - (ptr - buffer) < req->args.w_args.count) return -1;

            memcpy(req->args.w_args.buf, ptr, req->args.w_args.count);
            ptr += req->args.w_args.count;
            break;
        }

        case RPC_READ: {
            if (buffer_len - (ptr - buffer) < (sizeof(uint32_t) * 2)) return -1;

            uint32_t net_fd, net_count;
            
            memcpy(&net_fd, ptr, sizeof(uint32_t));
            req->args.w_args.fd = (int)be32toh(net_fd);
            ptr += sizeof(uint32_t);

            memcpy(&net_count, ptr, sizeof(uint32_t));
            req->args.w_args.count = be32toh(net_count);
            ptr += sizeof(uint32_t);
            
            break;
        }

        // TUTAJ obsługa pozostałych opcode'ów (read, lseek, itd.)
    }

    return 0;
}

ssize_t marshal_response(uint8_t opcode, const rpc_response_t *resp, uint8_t *buffer, size_t buffer_size) {
    if (buffer_size < 20) return -1;
    uint8_t *ptr = buffer;
    
    uint64_t net_seq = htobe64(resp->seq_number);
    memcpy(ptr, &net_seq, sizeof(uint64_t));
    ptr += sizeof(uint64_t);

    int32_t net_status = htobe32((uint32_t)resp->status);
    memcpy(ptr, &net_status, sizeof(int32_t));
    ptr += sizeof(int32_t);

    int64_t net_retval = htobe64((uint64_t)resp->return_value);
    memcpy(ptr, &net_retval, sizeof(int64_t));
    ptr += sizeof(int64_t);

    if (opcode == RPC_READ && resp->return_value>0){
        if(buffer_size - (ptr - buffer) < resp->return_value) return -1;
        memcpy(ptr, resp->data, resp->return_value);
        ptr += resp->return_value;
    }

    return (ssize_t)(ptr - buffer);
}

int unmarshal_response(uint8_t opcode, const uint8_t *buffer, size_t buffer_len, rpc_response_t *resp) {
    if (buffer_len < 20) return -1;
    const uint8_t *ptr = buffer;

    uint64_t net_seq; memcpy(&net_seq, ptr, sizeof(uint64_t)); ptr += sizeof(uint64_t);
    resp->seq_number = be64toh(net_seq);

    int32_t net_status; memcpy(&net_status, ptr, sizeof(int32_t)); ptr += sizeof(int32_t);
    resp->status = (int32_t)be32toh((uint32_t)net_status);

    int64_t net_retval; memcpy(&net_retval, ptr, sizeof(int64_t));
    resp->return_value = (int64_t)be64toh((uint64_t)net_retval);

    if (opcode == RPC_READ && resp->return_value>0){
        if(buffer_len - (ptr - buffer) < resp->return_value) return -1;
        memcpy(resp->data, ptr, resp->return_value);
        ptr += resp->return_value;
    }

    return 0;
}