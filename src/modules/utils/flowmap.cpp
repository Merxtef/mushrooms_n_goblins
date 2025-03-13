#include "flowmap.hpp"

#include <raymath.h>

#include <cstdio>

void gen_flow_map(Vector2* res, char* dijkstramap, size_t w, size_t h)
{

  const int dx[] = {-1, 1, 0, 0, 1,  1, -1, -1};
  const int dy[] = {0, 0, -1, 1, 1, -1,  1, -1};

  for (int y = 1; y < h - 1; ++y)
    for (int x = 1; x < w - 1; ++x)
    {
      if (dijkstramap[y * w + x] == 0)
      {
        res[y * w + x] = { 0.f, 0.f };
        continue;
      }

      Vector2 accum = { 0.f, 0.f };
      char last_min = 127;

      for (int i = 0; i < 8; ++i)
      {
        int xx = x + dx[i];
        int yy = y + dy[i];
        size_t idx = yy * w + xx;

        if (dijkstramap[idx] == 127)
          continue;

        if (last_min == dijkstramap[idx])
        {
          accum = Vector2Add(accum, { (float) dx[i], (float) dy[i] });
          // accum = { (float) dx[i], (float) dy[i] };
        }
        else if (last_min > dijkstramap[idx])
        {
          accum = { (float) dx[i], (float) dy[i] };
          last_min = dijkstramap[idx];
        }
      }

      // std::printf("(%f, %f) -> ", accum.x, accum.y);

      res[y * w + x] = Vector2Normalize(accum);

      // std::printf("(%f, %f)\n", res[y * w + x].x, res[y * w + x].y);
    }
}