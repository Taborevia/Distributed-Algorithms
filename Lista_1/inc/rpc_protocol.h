#ifndef RPC_PROTOCOL_H
#define RPC_PROTOCOL_H

#include <stdint.h>
#include <sys/types.h>

/* --- KONFIGURACJA PROTOKOŁU --- */
#define MAX_CHUNK_SIZE 4096  // Maksymalna ilość danych (np. dla read/write) w jednym żądaniu UDP
#define MAX_PATH_LEN 256     // Maksymalna długość ścieżki pliku
#define MAX_MODE_LEN 8       // Maksymalna długość łańcucha trybu (np. "r+", "w")

/* --- KODY OPERACJI (OPCODES) --- */
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

/* --- STRUKTURA ŻĄDANIA (KLIENT -> SERWER) --- */
typedef struct {
    uint8_t opcode;          // Kod operacji z rpc_opcode_t
    uint64_t auth_token;     // 64-bitowy token uwierzytelniający
    uint64_t seq_number;     // Numer sekwencyjny żądania (do obsługi powtórzeń)

    // Unia z argumentami - w pamięci zajmie tyle, co największa z poniższych struktur
    union {
        // Argumenty dla: open(const char *pathname, char *mode)
        struct {
            char pathname[MAX_PATH_LEN];
            char mode[MAX_MODE_LEN]; 
        } open_args;

        // Argumenty dla write
        struct {
            int fd;                       // Zdalny deskryptor pliku
            uint32_t count;               // Ile bajtów chcemy odczytać/zapisać
            uint8_t buf[MAX_CHUNK_SIZE];  // Bufor z danymi (tylko dla operacji WRITE)
        } w_args;

        // Argumenty dla read
        struct {
            int fd;                       // Zdalny deskryptor pliku
            uint32_t count;               // Ile bajtów chcemy odczytać/zapisać
        } r_args;

        // Argumenty dla: lseek(off_t offset, int whence, File *f)
        struct {
            int fd;
            int64_t offset;  // off_t może różnić się wielkością, wymuszamy 64-bity
            int whence;
        } lseek_args;

        // Argumenty dla: close(File *f)
        struct {
            int fd;
        } close_args;

        // Argumenty dla: chmod(const char *pathname, mode_t mode)
        struct {
            char pathname[MAX_PATH_LEN];
            uint32_t mode;   // mode_t ustandaryzowane do 32-bitów
        } chmod_args;

        // Argumenty dla: unlink(const char *pathname)
        struct {
            char pathname[MAX_PATH_LEN];
        } unlink_args;

        // Argumenty dla: rename(const char *oldpath, const char *newpath)
        struct {
            char oldpath[MAX_PATH_LEN];
            char newpath[MAX_PATH_LEN];
        } rename_args;

    } args;
} rpc_request_t;

/* --- STRUKTURA ODPOWIEDZI (SERWER -> KLIENT) --- */
typedef struct {
    uint64_t seq_number;          // Musi odpowiadać seq_number z żądania
    int32_t status;               // 0 = SUKCES, < 0 = BŁĄD (najlepiej zwracać -errno)
    int64_t return_value;         // Standardowa wartość zwracana (np. fd z open, rozmiar z read/write)
    
    // Dane przesyłane z powrotem do klienta (tylko dla operacji READ)
    uint8_t data[MAX_CHUNK_SIZE]; 
} rpc_response_t;

/* --- STRUKTURA WEWNĘTRZNA KLIENTA (Nie leci przez sieć!) --- */
// Ta struktura realizuje wymóg zadania zwracania typu `File *`
typedef struct {
    int fd;                 // Otrzymany od serwera "prawdziwy" deskryptor
    uint64_t auth_token;    // Przydatne, by biblioteka miała token zawsze pod ręką
} File;

#endif // RPC_PROTOCOL_H