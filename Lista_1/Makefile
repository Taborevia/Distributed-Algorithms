# --- Konfiguracja Kompilatora ---
CC = gcc
# -I$(INC_DIR) mówi kompilatorowi, żeby szukał plików .h w folderze inc
# -Wall -Wextra włącza ostrzeżenia, które pomagają wyłapać błędy
# -g dodaje symbole debugowania (przydatne np. w gdb lub valgrind)
CFLAGS = -Wall -Wextra -g -I$(INC_DIR)

# --- Foldery ---
SRC_DIR = src
INC_DIR = inc

# --- Pliki Wyjściowe (Programy) ---
SERVER_BIN = rpc_server
CLIENT_BIN = rpc_client

# --- Pliki Źródłowe ---
# Marshaling jest współdzielony przez oba programy
SERVER_SRCS = $(SRC_DIR)/server.c $(SRC_DIR)/marshaling.c
CLIENT_SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/client.c $(SRC_DIR)/marshaling.c

# --- Reguły Główne ---
# Uruchomienie 'make' bez argumentów wywoła regułę 'all'
.PHONY: all clean

all: $(SERVER_BIN) $(CLIENT_BIN)

# --- Reguła kompilacji serwera ---
# $@ to nazwa celu (rpc_server)
# $^ to wszystkie pliki zależne (SERVER_SRCS)
$(SERVER_BIN): $(SERVER_SRCS)
	$(CC) $(CFLAGS) -o $@ $^

# --- Reguła kompilacji klienta ---
$(CLIENT_BIN): $(CLIENT_SRCS)
	$(CC) $(CFLAGS) -o $@ $^

# --- Czyszczenie ---
# Uruchom 'make clean', aby usunąć skompilowane pliki
clean:
	rm -f $(SERVER_BIN) $(CLIENT_BIN)