import sys

year = sys.argv[1]
for line in sys.stdin:
    line = line.strip()
    if not line:
        continue
    parts = line.split(",")
    station, date, type, value = parts[0], parts[1], parts[2], parts[3]
    if value == "9999" or value == '':
        continue
    if date.startswith(year):
        print(f"{date[4:6]}\t{station},{type},{value}")
