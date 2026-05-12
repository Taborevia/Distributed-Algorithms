#!/usr/bin/env python3
import sys

def combiner():
    current_pair = None

    for line in sys.stdin:
        line = line.strip()
        if not line:
            continue

        # Parsowanie wejścia z mappera
        word, doc_id = line.split('\t', 1)
        
        # Emitujemy parę tylko raz dla danego mappera (usuwanie lokalnych duplikatów)
        if current_pair != (word, doc_id):
            print(f"{word}\t{doc_id}")
            current_pair = (word, doc_id)

if __name__ == "__main__":
    combiner()
