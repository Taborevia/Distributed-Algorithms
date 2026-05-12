import sys

list_A = []
list_B = []
j = None

for line in sys.stdin:
    line = line.strip()
    if not line:
        continue

    key, value = line.split("\t", 1)
    matrix, i, val = value.split(",")

    if key != j:
        if j is not None:
            for a in list_A:
                for b in list_B:
                    print(f"{a[0]},{b[0]}\t{float(a[1]) * float(b[1])}")
        list_A = []
        list_B = []
        j = key

    if matrix == 'A':
        list_A.append([i, val])
    else: # matrix == 'B'
        list_B.append([i, val])

if j is not None:
    for a in list_A:
        for b in list_B:
            print(f"{a[0]},{b[0]}\t{float(a[1]) * float(b[1])}")
