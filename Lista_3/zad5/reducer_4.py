import sys

ratings = {}
old_user_id = None

for line in sys.stdin:
    line = line.strip()
    if not line:
        continue
    
    user_id, values_str = line.split("\t")
    values = values_str.split(",")
    
    movie_id = values[0]
    rating_val = float(values[1])
    sim_val = float(values[2])
    
    if user_id != old_user_id:
        if old_user_id is not None:
            for m_id, (r_sum, s_sum) in ratings.items():
                if s_sum > 0:
                    print(f"{old_user_id}\t{m_id},{r_sum / s_sum:.2f}")
        old_user_id = user_id
        ratings = {}
        
    if movie_id in ratings:
        ratings_sum, similarity_sum = ratings[movie_id]
        ratings[movie_id] = (ratings_sum + (rating_val * sim_val), similarity_sum + sim_val)
    else:
        ratings[movie_id] = (rating_val * sim_val, sim_val)

if old_user_id is not None:
    for m_id, (r_sum, s_sum) in ratings.items():
        if s_sum > 0:
            print(f"{old_user_id}\t{m_id},{r_sum / s_sum:.2f}")