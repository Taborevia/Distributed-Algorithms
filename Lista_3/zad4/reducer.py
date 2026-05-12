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
        if counter > 0:
            print(f"{current_month}\t{(sum/counter)/10:.2f}\t{min/10:.2f}\t{max/10:.2f}")
        current_month = month
        counter = 0
        sum = 0
        min = 9999
        max = -9999
    temp_counter, temp_sum, temp_min, temp_max = map(float, values.split(","))
    temp_counter = int(temp_counter)    
    counter += temp_counter
    sum += temp_sum
    if temp_min < min:
        min = temp_min
    if temp_max > max:
        max = temp_max
if counter > 0:
    print(f"{current_month}\t{(sum/counter)/10:.2f}\t{min/10:.2f}\t{max/10:.2f}")