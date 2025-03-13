#include "dijkstramap.hpp"

#include <queue>
#include <cstdio> //tmp

#include <utils/math.hpp>
#include <dungeon/types.hpp>

void gen_dijkstra_map(char* res, char* tiles, size_t w, size_t h, char source)
{
  std::queue<IVec2> q;

  for (int y = 0; y < h; ++y)
    for (int x = 0; x < w; ++x)
    {
      size_t idx = y * w + x;
      if (tiles[idx] == source)
      {
        res[idx] = 0;
        q.push(IVec2{ x, y });
      }
      else
      {
        res[idx] = 127;
      }
    }
  
  const int dx[] = {-1, 1, 0, 0, 1,  1, -1, -1};
  const int dy[] = {0, 0, -1, 1, 1, -1,  1, -1};

  while (!q.empty())
  {
    IVec2 current = q.front();
    q.pop();

    for (int i = 0; i < 8; ++i)
    {
      int nx = current.x + dx[i];
      int ny = current.y + dy[i];

      if (nx < 0 || nx >= w || ny < 0 || ny >= h)
        continue;
      
      size_t nidx = ny * w + nx;
      size_t idx = current.y * w + current.x;

      if (source != dungeon::wall && tiles[nidx] == dungeon::wall)
        continue;
      
      if (res[nidx] > res[idx] + 1)
      {
        res[nidx] = res[idx] + 1;

        q.push({ nx, ny });
      }
    }
  }
}