import sys

ratings = []
similarities = []
old_item_id = None

for line in sys.stdin:
    line = line.strip()
    if not line:
        continue
    
    item_id, values = line.split("\t")
    values = values.split(",")
    if (item_id != old_item_id):
        if old_item_id is not None:
            for rating in ratings:
                for similarity in similarities:
                        if float(similarity[1]) > 0:
                            print(f"{rating[0]}\t{similarity[0]},{rating[1]},{similarity[1]}")
        old_item_id = item_id
        similarities = []
        ratings = []

    if values[2] == "R":
        ratings.append(values)
    if values[2] == "S":
        similarities.append(values)
if old_item_id is not None:
    for rating in ratings:
        for similarity in similarities:
            print(f"{rating[0]}\t{similarity[0]},{rating[1]},{similarity[1]}")
        
    