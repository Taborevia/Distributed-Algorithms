#!/usr/bin/env python3
import sys
import os

def load_vector(filename):
    vector = {}
    # Sprawdzamy, czy plik istnieje w bieżącym katalogu roboczym
    if os.path.exists(filename):
        with open(filename, 'r') as f:
            for line in f:
                # Format wektora: V,indeks,wartość
                parts = line.strip().split(',')
                if len(parts) == 3 and parts[0] == 'V':
                    idx = int(parts[1])
                    val = float(parts[2])
                    vector[idx] = val
    return vector

# Wczytujemy wektor raz na starcie Mappera
vector_v = load_vector('vector.txt')

for line in sys.stdin:
    # Format macierzy: M,wiersz,kolumna,wartość
    line = line.strip()
    if not line:
        continue
        
    parts = line.split(',')
    if parts[0] == 'M':
        row_i = parts[1] # Klucz (wiersz) - zostawiamy jako string
        col_j = int(parts[2])
        m_val = float(parts[3])
        
        # Mnożenie częściowe
        if col_j in vector_v:
            partial_product = m_val * vector_v[col_j]
            # Hadoop Streaming oczekuje formatu: klucz\twartość
            print(f"{row_i}\t{partial_product}")
