import sys
import ast

for line in sys.stdin:
    line = line.strip()
    if not line:
        continue
    
    items_id, similarity = line.split("\t")
    item_1, item_2 = items_id.split(",")
    print(f"{item_1}\t{item_2},{similarity},S")
            