#include "math.hpp"

#include <assert.h>

IVec2 getCellAt(Vector2 position)
{
  return {
    (int) (position.x / Settings::TILE_SIZE),
    (int) (position.y / Settings::TILE_SIZE)
  };
}

std::array<IVec2, 4> getCellsAt(Vector2 position)
{
  int x = (int) (position.x / Settings::TILE_SIZE - 0.5f);
  int y = (int) (position.y / Settings::TILE_SIZE - 0.5f);

  assert(0 <= x && x < Settings::DUNGEON_WIDTH - 1, "x is out of bounds");
  assert(0 <= y && y < Settings::DUNGEON_HEIGHT - 1, "y is out of bounds");

  return {
    IVec2{ x,     y },
    IVec2{ x + 1, y },
    IVec2{ x,     y + 1 },
    IVec2{ x + 1, y + 1 }
  };
}

IVec2 getClosestAdjacentCellAt(Vector2 position)
{
  int x = (int) (position.x / Settings::TILE_SIZE);
  int y = (int) (position.y / Settings::TILE_SIZE);

  assert(0 < x && x < Settings::DUNGEON_WIDTH - 1, "x is out of bounds");
  assert(0 < y && y < Settings::DUNGEON_HEIGHT - 1, "y is out of bounds");

  float inTileX = position.x / Settings::TILE_SIZE - x;
  float inTileY = position.y / Settings::TILE_SIZE - y;

  // This basically maps four triangle of a tile to number 0-3
  // up    => x > y && x + y < 1 => 0
  // right => x > y && x + y > 1 => 1
  // left  => x < y && x + y < 1 => 2
  // down  => x < y && x + y > 1 => 3
  int segment = (inTileX < inTileY) * 2 + (inTileX + inTileY > 1);

  switch (segment)
  {
  case 0: // up
    return { x, y - 1 };
  
  case 1: // right
    return { x + 1, y };
  
  case 2: // left
    return { x - 1, y };
  
  case 3: // down
    return { x, y + 1 };
  }
}

IVec2 getClosestCornerCellAt(Vector2 position)
{
  int x = (int) (position.x / Settings::TILE_SIZE);
  int y = (int) (position.y / Settings::TILE_SIZE);

  assert(0 < x && x < Settings::DUNGEON_WIDTH - 1, "x is out of bounds");
  assert(0 < y && y < Settings::DUNGEON_HEIGHT - 1, "y is out of bounds");

  float inTileX = position.x / Settings::TILE_SIZE - x;
  float inTileY = position.y / Settings::TILE_SIZE - y;

  if (inTileX < 0.5f)
    return (inTileY < 0.5f) ? IVec2{ x - 1, y - 1 } : IVec2{ x - 1, y + 1 };
  else
    return (inTileY < 0.5f) ? IVec2{ x + 1, y - 1 } : IVec2{ x + 1, y + 1 };
}

Vector3 getHeatmapColor(char value)
{
  constexpr Vector3 yellowWhite = {220, 220, 50};
  constexpr Vector3 red = {220, 0, 0};
  constexpr Vector3 purple = {74, 10, 116};
  constexpr Vector3 blue = {30, 30, 120};

  constexpr float pivotRed = 2.f;
  constexpr float pivotPurple = 4.f;
  constexpr float pivotBlue = 6.f;

  if (value <= 0)
    return yellowWhite;
  
  if (value <= pivotRed)
    return Vector3Lerp(yellowWhite, red, value / pivotRed);
  
  if (value <= pivotPurple)
    return Vector3Lerp(red, purple, (value - pivotRed) / (pivotPurple - pivotRed));
  
  if (value <= pivotBlue)
    return Vector3Lerp(purple, blue, (value - pivotPurple) / (pivotBlue - pivotPurple));

  return blue;
}