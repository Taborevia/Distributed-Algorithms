import sys

ratings = []
current_user = None
for line in sys.stdin:
    line = line.strip()
    if not line:
        continue
    user, values = line.split("\t")
    if current_user is None:
        current_user = user
    if user != current_user:
        if len(ratings) > 0:
            print(f"{current_user}\t{ratings}")
        current_user = user
        ratings = []
    values = values.split(",")
    ratings.append(values)
if len(ratings) > 0:
    print(f"{current_user}\t{ratings}")