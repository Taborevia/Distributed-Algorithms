import sys
import ast

for line in sys.stdin:
    line = line.strip()
    if not line:
        continue
    
    user_id, list_as_string = line.split("\t")
    
    ratings_list = ast.literal_eval(list_as_string)
    
    for item_pair in ratings_list:
        if len(item_pair) == 2:
            item_id = item_pair[0]
            rating = item_pair[1]
            
            print(f"{item_id}\t{user_id},{rating},R")
            