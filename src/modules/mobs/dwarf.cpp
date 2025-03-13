#include "dwarf.hpp"

#include <utils/math.hpp>
#include <dungeon/types.hpp>
#include <random>

void create_dwarf(entt::registry& registry, Position position)
{
  entt::entity dwarfEntity = registry.create();
  registry.emplace<Dwarf>(dwarfEntity);
  registry.emplace<Position>(dwarfEntity, position);
  registry.emplace<Velocity>(dwarfEntity, Vector2{ 2.0f, 0.0f }, 32.f);
  registry.emplace<SteerDir>(dwarfEntity);
  registry.emplace<SteerAccel>(dwarfEntity, 16.f);
}

void DwarfSystem::update(entt::registry& registry, AssetManager& mgr, float dt)
{
  static const auto dwarfView = registry.view<Dwarf, Position>();
  static const auto dwarfSteerView = registry.view<Dwarf, Position, Velocity, SteerDir>();
  static const auto dungeonView = registry.view<DungeonData>();

  entt::entity dungeonEntity = dungeonView.front();
  DungeonData& dd = registry.get<DungeonData>(dungeonEntity);

  for (auto&& [entity, dwarf, p]: dwarfView.each())
  {
    auto curCell = getCellAt(p);
    size_t curCellIdx = curCell.y * dd.width + curCell.x;

    float mushTreeDist = dd.mushroomTreeDM[curCellIdx];

    // Stats degrade
    float mult = (dwarf.state == WallDigging) ? Settings::hungerWorkMult : 1.f;
    dwarf.hunger += dwarf.hungerSpeed * dt * mult;
    
    if (dwarf.state == Resting)
      mult = (mushTreeDist <= 1.001f) ? -Settings::restSpeedTree : -Settings::restSpeed;
    else
      mult = (dwarf.state == WallDigging) ? Settings::fatigueWorkMult : 1.f;
    
    dwarf.fatigue += dwarf.fatigueSpeed * dt * mult;

    dwarf.hunger = std::min(100.f, dwarf.hunger);
    dwarf.fatigue = std::min(100.f, dwarf.fatigue);

    if (dwarf.fatigue >= 99.99f)
      dwarf.state = Resting;

    if (dwarf.state == Resting && dwarf.fatigue > 0.1f && (dwarf.hunger < 95.f || dwarf.fatigue > 90.f))
      continue;  // Rest until fatigue restored

    auto closestACoord = getClosestAdjacentCellAt(p);
    size_t closestAIndex = closestACoord.y * dd.width + closestACoord.x;

    auto closestCells = getCellsAt(p);

    // Pushing out
    for (auto& tileCoord: closestCells)
    {
      if (dd.tiles[tileCoord.y * dd.width + tileCoord.x] != dungeon::wall && dd.tiles[tileCoord.y * dd.width + tileCoord.x] != dungeon::border)
        continue;

      float dist = pointToTileDist(p, tileCoord) - 15.9f;
      
      if (dist < 0.f)
      {
        Vector2 dir = Vector2Normalize(Vector2Subtract(getCellCenter(tileCoord), p));
        p = Position{Vector2Add(p, Vector2Scale(dir, dist))}; // Note: dir is p -> tileCenter, but dist is negative
      }
    }


    if (dwarf.state == WallDigging)
    {
      dwarf.wallDiggingTimer += dt;

      if (dwarf.wallDiggingTimer >= dwarf.wallDiggingTotalTime)
      {
        for (auto& tileCoord: closestCells)
        {
          size_t idx = tileCoord.y * dd.width + tileCoord.x;
          if(dd.tiles[idx] == dungeon::wall)
          {
            dd.tiles[idx] = dungeon::floor;

            if (rand() % 1000 < 1000 * Settings::dwarfSpawnChance)
            {
              create_dwarf(registry, getCellCenter(tileCoord));
            }

            break;
          }
        }

        dwarf.wallDiggingTimer = 0.f;
        dwarf.state = WallSearch;
      }
      else
        continue; // Assume dwarfs are always to finish started job
    }
    else
    {
      for (auto& tileCoord: closestCells) // Alaways eat all mushrooms in immediate proximity
      {
        size_t idx = tileCoord.y * dd.width + tileCoord.x;
        if (dd.tiles[idx] != dungeon::mushroom)
          continue;

        dd.tiles[idx] = dungeon::floor;
        dwarf.hunger = std::max(dwarf.hunger - 40.f, 0.f);
      }
    }

    float mushDist = dd.mushroomDM[curCellIdx];

    float wallSearchUtility = (100.f - dwarf.hunger) * (100 - dwarf.fatigue) / 2.f;
    float mushroomSearchUtility = (dwarf.hunger * dwarf.hunger) / (30.f + 12.f * mushDist);
    float mushroomTreeSearchUtility =
      (dwarf.fatigue * dwarf.fatigue * dwarf.fatigue) * ((mushTreeDist - 8.f) * (mushTreeDist - 8.f) + 50.f)
      / 300000.f + (dwarf.fatigue > 60.f) * 40 + (dwarf.fatigue > 80.f) * 100;

    if (wallSearchUtility > mushroomSearchUtility && wallSearchUtility > mushroomTreeSearchUtility)
    {
      dwarf.state = WallSearch;
    }
    else if (mushroomSearchUtility > mushroomTreeSearchUtility)
    {
      dwarf.state = MushroomSearch;
    }
    else
    {
      dwarf.state = MushroomTreeSearch;
    }
  }

  // Update steering
  for (auto&& [entity, dwarf, p, v, sd]: dwarfSteerView.each())
  {
    if (dwarf.state == WallDigging || dwarf.state == Resting)
    {
      sd = { 0.f, 0.f };
      v.vector = { 0.f, 0.f };
      continue;
    }

    auto closestCells = getCellsAt(p);

    bool flag = false;
    switch (dwarf.state)
    {
      case WallSearch:
        for (auto& tileCoord: closestCells)
        {
          size_t idx = tileCoord.y * dd.width + tileCoord.x;
          if(dd.tiles[idx] == dungeon::wall)
          {
            sd = { 0.f, 0.f };
            v.vector = { 0.f, 0.f };
            dwarf.state = WallDigging;
          }
        }

        if (dwarf.state != WallDigging)
          sd = dirAccordingFlow(dd.wallFM, dd, p);
        
        break;

      case MushroomSearch:
        sd = dirAccordingFlow(dd.mushroomFM, dd, p);
        break;
      
      case MushroomTreeSearch:
        for (auto& tileCoord: closestCells)
        {
          size_t idx = tileCoord.y * dd.width + tileCoord.x;
          if(dd.tiles[idx] == dungeon::mushroomTree)
          {
            sd = { 0.f, 0.f };
            v.vector = { 0.f, 0.f };
            dwarf.state = Resting;
          }
        }

        if (dwarf.state != Resting)
          sd = dirAccordingFlow(dd.mushroomFM, dd, p);
        
        break;
    }
  }
}

SteerDir DwarfSystem::dirAccordingFlow(const std::vector<Vector2>& flowMap, const DungeonData& dd, Position position)
{
  auto cells = getCellsAt(position);
  size_t idxTL = cells[0].y * dd.width + cells[0].x;
  size_t idxTR = cells[1].y * dd.width + cells[1].x;
  size_t idxDL = cells[2].y * dd.width + cells[2].x;
  size_t idxDR = cells[3].y * dd.width + cells[3].x;

  return {
    biLerp(
      flowMap[idxTL], flowMap[idxTR], flowMap[idxDL], flowMap[idxDR],
      position.x / Settings::TILE_SIZE - cells[0].x,
      position.y / Settings::TILE_SIZE - cells[0].y
    )
  };
}
