import heapq
import sys

maze_path = sys.argv[1] if len(sys.argv) > 1 else 'mazes/maze20.txt'
lines = [l.split() for l in open(maze_path, encoding='utf-8').read().splitlines() if l.strip()]
rows, cols = map(int, lines[0])
g = [list(map(int, r)) for r in lines[1:]]
COST = {0: 1, 2: 3, 3: 1000, -1: 1, -2: 1}
start = end = None
for r in range(rows):
    for c in range(cols):
        if g[r][c] == -1:
            start = (r, c)
        if g[r][c] == -2:
            end = (r, c)
inf = float('inf')
dist = [[inf] * cols for _ in range(rows)]
prev = [[None] * cols for _ in range(rows)]
dist[start[0]][start[1]] = 0
pq = [(0, start)]
while pq:
    d, (r, c) = heapq.heappop(pq)
    if d > dist[r][c]:
        continue
    for dr, dc in ((-1, 0), (1, 0), (0, -1), (0, 1)):
        nr, nc = r + dr, c + dc
        if 0 <= nr < rows and 0 <= nc < cols and g[nr][nc] != 1:
            nd = d + COST[g[nr][nc]]
            if nd < dist[nr][nc]:
                dist[nr][nc] = nd
                prev[nr][nc] = (r, c)
                heapq.heappush(pq, (nd, (nr, nc)))
path = [end]
while path[-1] != start:
    path.append(prev[path[-1][0]][path[-1][1]])
path = path[::-1]
moves = ''
for (r1, c1), (r2, c2) in zip(path, path[1:]):
    moves += 'U' if r2 == r1 - 1 else 'D' if r2 == r1 + 1 else 'L' if c2 == c1 - 1 else 'R'
print('maze', maze_path)
print('steps', len(moves), 'cost', int(dist[end[0]][end[1]]))
print('moves', moves)
