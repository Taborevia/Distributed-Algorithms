#ifndef RPC_PROTOCOL_H
#define RPC_PROTOCOL_H

#include <stdint.h>
#include <sys/types.h>

#define MAX_CHUNK_SIZE 4096  
#define MAX_PATH_LEN 256     
#define MAX_MODE_LEN 8       

//opcodes
typedef enum {
    RPC_OPEN = 1,
    RPC_READ,
    RPC_WRITE,
    RPC_LSEEK,
    RPC_CLOSE,
    RPC_CHMOD,
    RPC_UNLINK,
    RPC_RENAME
} rpc_opcode_t;

// REQUEST STRUCTURE (CLIENT -> SERVER)
typedef struct {
    uint8_t opcode;          
    uint64_t auth_token;
    uint64_t seq_number;

    union {
        // open(const char *pathname, char *mode)
        struct {
            char pathname[MAX_PATH_LEN];
            char mode[MAX_MODE_LEN]; 
        } open_args;

        // Write(void *buf, size_t count, File *f)
        struct {
            int fd;                       // file descriptor
            uint32_t count;               // how many bytes to write 
            uint8_t buf[MAX_CHUNK_SIZE];  // data
        } w_args;

        // Read(void *buf, size_t count, File *f)
        struct {
            int fd;                       // file descriptor
            uint32_t count;               // how many bytes to read 
        } r_args;

        // seek(off_t offset, int whence, File *f)
        struct {
            int fd;
            int64_t offset;             // move by this many bytes
            int whence;
        } lseek_args;

        // close(File *f)
        struct {
            int fd;                 //file descriptor
        } close_args;

        // chmod(const char *pathname, mode_t mode)
        struct {
            char pathname[MAX_PATH_LEN];
            uint32_t mode;
        } chmod_args;

        // unlink(const char *pathname)
        struct {
            char pathname[MAX_PATH_LEN];
        } unlink_args;

        // ename(const char *oldpath, const char *newpath)
        struct {
            char oldpath[MAX_PATH_LEN];
            char newpath[MAX_PATH_LEN];
        } rename_args;

    } args;
} rpc_request_t;

// response structure (SERVER -> CLIENT)
typedef struct {
    uint64_t seq_number;          
    int32_t status;               // 0 = success, -1 = error
    int64_t return_value;         // fd for open, bytes read/written for read/write, etc.
    
    // data for read
    uint8_t data[MAX_CHUNK_SIZE]; 
} rpc_response_t;

#endif // RPC_PROTOCOL_H