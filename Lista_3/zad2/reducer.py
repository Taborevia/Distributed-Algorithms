#!/usr/bin/env python3
import sys

current_row = None
current_sum = 0

for line in sys.stdin:
    line = line.strip()
    if not line:
        continue
        
    row_i, p_val = line.split('\t')
    p_val = float(p_val)
    
    # Jeśli nadal jesteśmy w tym samym wierszu
    if current_row == row_i:
        current_sum += p_val
    else:
        # Jeśli to nowy wiersz, wypisujemy wynik poprzedniego
        if current_row is not None:
            print(f"{current_row}\t{current_sum}")
        
        current_row = row_i
        current_sum = p_val

# Wypisanie ostatniego wiersza
if current_row is not None:
    print(f"{current_row}\t{current_sum}")
