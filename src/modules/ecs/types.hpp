#pragma once

#include <vector>
#include <raylib.h>

#include <dungeon/types.hpp>
#include <settings/global_consts.hpp>

struct Position : Vector2 {};

struct DungeonData
{
  std::vector<char> tiles;

  // DijkstraMaps
  std::vector<char> wallDM;
  std::vector<char> mushroomDM;
  std::vector<char> mushroomTreeDM;

  // FlowMaps
  std::vector<Vector2> wallFM;
  std::vector<Vector2> mushroomFM;
  std::vector<Vector2> mushroomTreeFM;

  size_t width;
  size_t height;
};

struct TextureSource
{
  Texture2D tex;
};

struct IsPlayer {};

struct Tile
{
  size_t index;
  dungeon::DungeonTile value;
};

enum DwarfState
{
  WallSearch,
  MushroomSearch,
  MushroomTreeSearch,
  WallDigging,
  Resting,
};

// Too complex but given no scalability needed - will suffice
struct Dwarf
{
  float hunger = 0.f;
  float fatigue = 0.f;

  float hungerSpeed = Settings::hungerDecay;
  float fatigueSpeed = Settings::fatigueDecay;

  float wallDiggingTimer = 0.f;
  float wallDiggingTotalTime = 2.f;

  DwarfState state = WallSearch;
};

struct Velocity
{
  Vector2 vector = { 0.f, 0.f};
  float cap = 32.f;
};

struct SteerDir : Vector2 {};

struct SteerAccel
{
  float accel = 4.f;
};
