#pragma once

#include <array>
#include <raylib.h>
#include <raymath.h>

#include <ecs/types.hpp>
#include <settings/global_consts.hpp>

struct IVec2
{
  int x;
  int y;
};

inline bool operator==(const IVec2 &lhs, const IVec2 &rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }
inline bool operator!=(const IVec2 &lhs, const IVec2 &rhs) { return !(lhs == rhs); }

inline IVec2 operator-(const IVec2 &lhs, const IVec2 &rhs)
{
  return IVec2{lhs.x - rhs.x, lhs.y - rhs.y};
}

template<typename T>
inline T sqr(T a){ return a*a; }

template<typename T, typename U>
inline float dist_sq(const T &lhs, const U &rhs) { return float(sqr(lhs.x - rhs.x) + sqr(lhs.y - rhs.y)); }

template<typename T, typename U>
inline float dist(const T &lhs, const U &rhs) { return sqrtf(dist_sq(lhs, rhs)); }

IVec2 getCellAt(Vector2 position);
// Get 4 closest cells INCLUDING the one at 'position'
std::array<IVec2, 4> getCellsAt(Vector2 position);
IVec2 getClosestAdjacentCellAt(Vector2 position);
IVec2 getClosestCornerCellAt(Vector2 position);
inline Position getCellCenter(IVec2 position)
{
  return {
    position.x * Settings::TILE_SIZE + Settings::HALF_TILE_SIZE,
    position.y * Settings::TILE_SIZE + Settings::HALF_TILE_SIZE
  };
}
inline float pointToTileDist(Vector2 point, IVec2 tileCoords)
{
  return Vector2Distance(
    point,
    Vector2Clamp(
      point,
      Vector2{ tileCoords.x * Settings::TILE_SIZE, tileCoords.y * Settings::TILE_SIZE },
      Vector2{ (tileCoords.x + 1) * Settings::TILE_SIZE, (tileCoords.y + 1) * Settings::TILE_SIZE }
    )
  );
}

inline Vector2 biLerp(Vector2 a, Vector2 b, Vector2 c, Vector2 d, float v1, float v2)
{
  return Vector2Lerp(Vector2Lerp(a, b, v1), Vector2Lerp(c, d, v1), v2);
}

Vector3 getHeatmapColor(char value);

