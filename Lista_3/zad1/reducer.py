#!/usr/bin/env python3
import sys

def reducer():
    current_word = None
    documents = set()

    for line in sys.stdin:
        line = line.strip()
        if not line:
            continue

        try:
            word, doc_id = line.split('\t', 1)
        except ValueError:
            continue

        if current_word == word:
            documents.add(doc_id)
        else:
            if current_word:
                # Formatowanie wyjścia: słowo -> [doc1, doc2, ...]
                doc_list = ", ".join(sorted(list(documents)))
                print(f"{current_word}\t-> [{doc_list}]")
            
            current_word = word
            documents = {doc_id}

    # Obsługa ostatniego słowa
    if current_word:
        doc_list = ", ".join(sorted(list(documents)))
        print(f"{current_word}\t-> [{doc_list}]")

if __name__ == "__main__":
    reducer()
