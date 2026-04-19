#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "client.h"

void print_test_result(const char *test_name, int result) {
    if (result >= 0) {
        printf("[SUCCESS] %s (result: %d)\n", test_name, result);
    } else {
        printf("[ERROR]   %s (result: %d)\n", test_name, result);
    }
}

int main() {
    printf("=== Start ===\n");
    rpc_init("127.0.0.1", 8080);

    const char *file_orig = "rpc_test_file.txt";
    const char *file_renamed = "rpc_test_renamed.txt";
    char buffer[MAX_CHUNK_SIZE];
    const char *text_to_write = "To jest zawartosc pliku testowego RPC.";

    // --- TEST 1: OPEN and WRITE ---
    printf("\n--- Test 1: Open and write ---\n");
    File *f = rpc_open(file_orig, "w+");
    if (!f) {
        printf("OPEN ERROR\n");
        return 1;
    }
    ssize_t written = rpc_write((void*)text_to_write, strlen(text_to_write), f);
    print_test_result("rpc_write", (int)written);

    // --- TEST 2: LSEEK and READ ---
    printf("\n--- Test 2: LSEEK and READ---\n");
    rpc_lseek(f, 0, SEEK_SET); // move cursor to the beginning of the file
    memset(buffer, 0, MAX_CHUNK_SIZE);
    ssize_t read_bytes = rpc_read(buffer, MAX_CHUNK_SIZE, f);
    if (read_bytes > 0) {
        printf("Read: %.*s\n", (int)read_bytes, buffer);
    }
    print_test_result("rpc_read", (int)read_bytes);
    rpc_close(f);

    // --- TEST 3: CHMOD ---
    printf("\n--- Test 3: chmod 0444 - only read ---\n");
    int chmod_res = rpc_chmod(file_orig, 0444);
    print_test_result("rpc_chmod", chmod_res);

    // check if file is actually read-only by trying to open it for writing again
    File *f_check = rpc_open(file_orig, "w");
    if (f_check == NULL) {
        printf("Success Fail, cant write to file after chmod.\n");
    } else {
        printf("Error: Can write to file despite chmod.\n");
        rpc_close(f_check);
    }

    // --- TEST 4: RENAME ---
    printf("\n--- Test 4: RENAME ---\n");
    int rename_res = rpc_rename(file_orig, file_renamed);
    print_test_result("rpc_rename", rename_res);

    // --- TEST 5: UNLINK ---
    printf("\n--- Test 5: UNLINK ---\n");
    int unlink_res = rpc_unlink(file_renamed);
    print_test_result("rpc_unlink", unlink_res);

    printf("\n=== END ===\n");
    return 0;
}