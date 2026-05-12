import sys

counter = 0
sum = 0
min = 9999
max = -9999
current_month = None
for line in sys.stdin:
    line = line.strip()
    if not line:
        continue
    month, values = line.split("\t")
    if current_month is None:
        current_month = month
    if month != current_month:
        print(f"{current_month}\t{counter},{sum},{min},{max}")
        current_month = month
        counter = 0
        sum = 0
        min = 9999
        max = -9999
    station, type, value = values.split(",")
    value = int(value)
    counter += 1
    sum += value
    if value < min:
        min = value
    if value > max:
        max = value
print(f"{current_month}\t{counter},{sum},{min},{max}")
