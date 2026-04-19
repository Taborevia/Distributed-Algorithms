#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <endian.h>
#include "rpc_protocol.h"

ssize_t marshal_request(const rpc_request_t *req, uint8_t *buffer, size_t buffer_size) {
    if (!req || !buffer || buffer_size < sizeof(rpc_request_t)) {
        return -1; 
    }

    uint8_t *ptr = buffer;

    // 1. opcode 1 byte
    *ptr = req->opcode;
    ptr += sizeof(uint8_t);

    // 2. auth token (8 Bytes, conversion to Big-Endian)
    uint64_t net_auth = htobe64(req->auth_token);
    memcpy(ptr, &net_auth, sizeof(uint64_t));
    ptr += sizeof(uint64_t);

    // 3. seq_number (8 Bytes, conversion to Big-Endian)
    uint64_t net_seq = htobe64(req->seq_number);
    memcpy(ptr, &net_seq, sizeof(uint64_t));
    ptr += sizeof(uint64_t);

    // 4. arguments
    switch (req->opcode) {
        case RPC_OPEN:
            memcpy(ptr, req->args.open_args.pathname, MAX_PATH_LEN);
            ptr += MAX_PATH_LEN;
            memcpy(ptr, req->args.open_args.mode, MAX_MODE_LEN);
            ptr += MAX_MODE_LEN;
            break;

        case RPC_WRITE: {
            uint32_t net_fd = htobe32((uint32_t)req->args.w_args.fd);
            memcpy(ptr, &net_fd, sizeof(uint32_t));
            ptr += sizeof(uint32_t);

            uint32_t net_count = htobe32(req->args.w_args.count);
            memcpy(ptr, &net_count, sizeof(uint32_t));
            ptr += sizeof(uint32_t);

            memcpy(ptr, req->args.w_args.buf, req->args.w_args.count);
            ptr += req->args.w_args.count;
            break;
        }

        case RPC_LSEEK: {
            uint32_t net_fd = htobe32((uint32_t)req->args.lseek_args.fd);
            memcpy(ptr, &net_fd, sizeof(uint32_t));
            ptr += sizeof(uint32_t);

            uint64_t net_offset = htobe64((uint64_t)req->args.lseek_args.offset);
            memcpy(ptr, &net_offset, sizeof(uint64_t));
            ptr += sizeof(uint64_t);

            uint32_t net_whence = htobe32((uint32_t)req->args.lseek_args.whence);
            memcpy(ptr, &net_whence, sizeof(uint32_t));
            ptr += sizeof(uint32_t);
            break;
        }

        case RPC_READ: {
            uint32_t net_fd = htobe32((uint32_t)req->args.r_args.fd);
            memcpy(ptr, &net_fd, sizeof(uint32_t));
            ptr += sizeof(uint32_t);

            uint32_t net_count = htobe32(req->args.r_args.count);
            memcpy(ptr, &net_count, sizeof(uint32_t));
            ptr += sizeof(uint32_t);

            break;
        }

        case RPC_CLOSE: {
            uint32_t net_fd = htobe32((uint32_t)req->args.close_args.fd);
            memcpy(ptr, &net_fd, sizeof(uint32_t));
            ptr += sizeof(uint32_t);
            break;
        }

        case RPC_CHMOD: {
            memcpy(ptr, req->args.chmod_args.pathname, MAX_PATH_LEN);
            ptr += MAX_PATH_LEN;
            uint32_t net_mode = htobe32(req->args.chmod_args.mode);
            memcpy(ptr, &net_mode, sizeof(uint32_t));
            ptr += sizeof(uint32_t);
            break;
        }

        case RPC_UNLINK: {
            memcpy(ptr, req->args.unlink_args.pathname, MAX_PATH_LEN);
            ptr += MAX_PATH_LEN;
            break;
        }

        case RPC_RENAME: {
            memcpy(ptr, req->args.rename_args.oldpath, MAX_PATH_LEN);
            ptr += MAX_PATH_LEN;
            memcpy(ptr, req->args.rename_args.newpath, MAX_PATH_LEN);
            ptr += MAX_PATH_LEN;
            break;
        }


        default:
            break;
    }

    return (ssize_t)(ptr - buffer);
}


int unmarshal_request(const uint8_t *buffer, size_t buffer_size, rpc_request_t *req) {
    if (!buffer || !req || buffer_size < 17) {
        // 17 = 1 byte opcode + 8 bytes token + 8 bytes seq_number (minimum)
        return -1;
    }

    const uint8_t *ptr = buffer;

    // 1. opcode
    req->opcode = *ptr;
    ptr += sizeof(uint8_t);

    // 2. auth_token
    uint64_t net_auth;
    memcpy(&net_auth, ptr, sizeof(uint64_t));
    req->auth_token = be64toh(net_auth);
    ptr += sizeof(uint64_t);

    // 3. seq_number
    uint64_t net_seq;
    memcpy(&net_seq, ptr, sizeof(uint64_t));
    req->seq_number = be64toh(net_seq);
    ptr += sizeof(uint64_t);

    // 4. arguments
    switch (req->opcode) {
        case RPC_OPEN:
        
            if (buffer_size - (ptr - buffer) < (MAX_PATH_LEN + MAX_MODE_LEN)) return -1;
            
            memcpy(req->args.open_args.pathname, ptr, MAX_PATH_LEN);
            ptr += MAX_PATH_LEN;
            memcpy(req->args.open_args.mode, ptr, MAX_MODE_LEN);
            ptr += MAX_MODE_LEN;
            break;

        case RPC_WRITE: {
            if (buffer_size - (ptr - buffer) < (sizeof(uint32_t) * 2)) return -1;

            uint32_t net_fd, net_count;
            
            memcpy(&net_fd, ptr, sizeof(uint32_t));
            req->args.w_args.fd = (int)be32toh(net_fd);
            ptr += sizeof(uint32_t);

            memcpy(&net_count, ptr, sizeof(uint32_t));
            req->args.w_args.count = be32toh(net_count);
            ptr += sizeof(uint32_t);

            if (req->args.w_args.count > MAX_CHUNK_SIZE) return -1;
            if (buffer_size - (ptr - buffer) < req->args.w_args.count) return -1;

            memcpy(req->args.w_args.buf, ptr, req->args.w_args.count);
            ptr += req->args.w_args.count;
            break;
        }

        case RPC_LSEEK: {
            if (buffer_size - (ptr - buffer) < (sizeof(uint32_t) * 2 + sizeof(uint64_t))) return -1;

            uint32_t net_fd, net_whence;
            uint64_t net_offset;
            
            memcpy(&net_fd, ptr, sizeof(uint32_t));
            req->args.lseek_args.fd = (int)be32toh(net_fd);
            ptr += sizeof(uint32_t);

            memcpy(&net_offset, ptr, sizeof(uint64_t));
            req->args.lseek_args.offset = (int64_t)be64toh(net_offset);
            ptr += sizeof(uint64_t);

            memcpy(&net_whence, ptr, sizeof(uint32_t));
            req->args.lseek_args.whence = (int)be32toh(net_whence);
            ptr += sizeof(uint32_t);
            break;
        }

        case RPC_READ: {
            if (buffer_size - (ptr - buffer) < (sizeof(uint32_t) * 2)) return -1;

            uint32_t net_fd, net_count;
            
            memcpy(&net_fd, ptr, sizeof(uint32_t));
            req->args.r_args.fd = (int)be32toh(net_fd);
            ptr += sizeof(uint32_t);

            memcpy(&net_count, ptr, sizeof(uint32_t));
            req->args.r_args.count = be32toh(net_count);
            ptr += sizeof(uint32_t);
            
            break;
        }

        case RPC_CLOSE: {
            if (buffer_size - (ptr - buffer) < sizeof(uint32_t)) return -1;

            uint32_t net_fd;
            memcpy(&net_fd, ptr, sizeof(uint32_t));
            req->args.close_args.fd = (int)be32toh(net_fd);
            ptr += sizeof(uint32_t);
            break;
        }

        case RPC_CHMOD:
            memcpy(req->args.chmod_args.pathname, ptr, MAX_PATH_LEN);
            ptr += MAX_PATH_LEN;
            uint32_t net_mode;
            memcpy(&net_mode, ptr, sizeof(uint32_t));
            req->args.chmod_args.mode = be32toh(net_mode);
            ptr += sizeof(uint32_t);
            break;

        case RPC_UNLINK:
            memcpy(req->args.unlink_args.pathname, ptr, MAX_PATH_LEN);
            ptr += MAX_PATH_LEN;
            break;

        case RPC_RENAME:
            memcpy(req->args.rename_args.oldpath, ptr, MAX_PATH_LEN);
            ptr += MAX_PATH_LEN;
            memcpy(req->args.rename_args.newpath, ptr, MAX_PATH_LEN);
            ptr += MAX_PATH_LEN;
            break;

    }

    return 0;
}

ssize_t marshal_response(uint8_t opcode, const rpc_response_t *resp, uint8_t *buffer, int64_t buffer_size) {
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

int unmarshal_response(uint8_t opcode, const uint8_t *buffer, int64_t buffer_size, rpc_response_t *resp) {
    if (buffer_size < 20) return -1;
    const uint8_t *ptr = buffer;

    uint64_t net_seq; memcpy(&net_seq, ptr, sizeof(uint64_t));
    ptr += sizeof(uint64_t);
    resp->seq_number = be64toh(net_seq);

    int32_t net_status; memcpy(&net_status, ptr, sizeof(int32_t));
    ptr += sizeof(int32_t);
    resp->status = (int32_t)be32toh((uint32_t)net_status);

    int64_t net_retval; memcpy(&net_retval, ptr, sizeof(int64_t));
    ptr += sizeof(int64_t);
    resp->return_value = (int64_t)be64toh((uint64_t)net_retval);

    if (opcode == RPC_READ && resp->return_value>0){
        if(buffer_size - (ptr - buffer) < resp->return_value) return -1;
        memcpy(resp->data, ptr, resp->return_value);
        ptr += resp->return_value;
    }

    return 0;
}