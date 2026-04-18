#include <stdio.h>
#include <string.h>

#include "rpc_protocol.h"

// Zewnętrzne deklaracje z biblioteki klienta
extern void rpc_init(const char *server_ip, int port);
extern File *rpc_open(const char *pathname, char *mode);
extern ssize_t rpc_write(void *buf, size_t count, File *f);

int main() {
    printf("Uruchamianie aplikacji klienta...\n");
    rpc_init("127.0.0.1", 8080);

    printf("Otwieranie pliku 'test_zdalny.txt'...\n");
    File *plik = rpc_open("test_zdalny.txt", "w");
    if (!plik) {
        printf("Błąd otwierania pliku!\n");
        return 1;
    }

    const char *wiadomosc = "To jest test zapisu przez wlasny mechanizm RPC i UDP!\n";
    printf("Zapisywanie danych...\n");
    
    ssize_t zapisano = rpc_write((void*)wiadomosc, strlen(wiadomosc), plik);
    if (zapisano > 0) {
        printf("Sukces! Zapisano %ld bajtów.\n", zapisano);
    } else {
        printf("Błąd zapisu.\n");
    }

    return 0;
}