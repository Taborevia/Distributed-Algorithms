import sys
import ast

for line in sys.stdin:
    line = line.strip()
    if not line:
        continue
    user, ratings = line.split("\t")
    ratings = ast.literal_eval(ratings)
    for i in range(len(ratings)):
        for j in range(i + 1, len(ratings)):
            print(f"{ratings[i][0]},{ratings[j][0]}\t{ratings[i][1]},{ratings[j][1]}")
