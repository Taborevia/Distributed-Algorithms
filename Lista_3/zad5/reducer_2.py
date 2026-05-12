import sys
import math

def calculate_similarity(n, s_x, s_y, s_xx, s_yy, s_xy):
    if n < 2:
        return None
    
    numerator = (n * s_xy) - (s_x * s_y)
    dom_x = (n * s_xx) - (s_x * s_x)
    dom_y = (n * s_yy) - (s_y * s_y)
    
    denominator = math.sqrt(max(0, dom_x * dom_y)) # max, by uniknąć błędów precyzji < 0
    
    if denominator == 0:
        return 0.0
    return numerator / denominator

old_films = None
sum_x = 0
sum_y = 0
sum_xx = 0
sum_yy = 0
sum_xy = 0
counter = 0

for line in sys.stdin:
    line = line.strip()
    if not line:
        continue
    films, ratings = line.split("\t")
    if old_films is None:
        old_films = films
    if films != old_films:
        res = calculate_similarity(counter, sum_x, sum_y, sum_xx, sum_yy, sum_xy)
        if res is not None:
            print(f"{old_films}\t{res:.4f}")
        old_films = films
        sum_x = 0
        sum_y = 0
        sum_xx = 0
        sum_yy = 0
        sum_xy = 0
        counter = 0
    x, y = map(float, ratings.split(","))
    sum_x += x
    sum_y += y
    sum_xx += x*x
    sum_yy += y*y
    sum_xy += x*y
    counter += 1
    
res = calculate_similarity(counter, sum_x, sum_y, sum_xx, sum_yy, sum_xy)
if res is not None:
    print(f"{old_films}\t{res:.4f}")
