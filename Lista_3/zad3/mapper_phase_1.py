import sys

for line in sys.stdin:
    line = line.strip()
    if not line:
        continue
    parts = line.split(",")
    if len(parts) != 4:
        continue
    matrix, i, j, value = parts
    if matrix == 'A':
        print(f"{j}\t{matrix},{i},{value}")
    else: # matrix == 'B'
        print(f"{i}\t{matrix},{j},{value}")
