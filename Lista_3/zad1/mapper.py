#!/usr/bin/env python3
import sys
import os
import re

def mapper():
    # Pobieramy nazwę pliku, z którego pochodzi linia
    input_file = os.environ.get('map_input_file', 'unknown_doc')
    # Wyciągamy samą nazwę pliku z pełnej ścieżki
    doc_id = os.path.basename(input_file)

    for line in sys.stdin:
        # Odporność na błędy: usuwanie białych znaków i obsługa pustych linii
        line = line.strip()
        if not line:
            continue
            
        # Usuwanie interpunkcji i zamiana na małe litery
        line = re.sub(r'[^\w\s]', '', line).lower()
        words = line.split()

        for word in words:
            # Emitujemy słowo i nazwę dokumentu
            print(f"{word}\t{doc_id}")

if __name__ == "__main__":
    mapper()
