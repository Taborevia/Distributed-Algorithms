import sys

last_key = None
sum = 0

for line in sys.stdin:
    line = line.strip()
    if not line:
        continue
    key, value = line.split("\t")
    if key != last_key:
        if last_key is not None:
            print(f"C,{last_key},{sum}")
        last_key = key
        sum = 0
    sum += float(value)
print(f"C,{last_key},{sum}")

