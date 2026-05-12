import sys

usersRatings = {}
for line in sys.stdin:
    line = line.strip()
    if not line:
        continue
    if line.startswith("userId"):
        continue
    userId, movieId, rating, timestamp = line.split(",")
    print(f"{userId}\t{movieId},{rating}")
