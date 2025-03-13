#include "dungeon_change.hpp"

#include <random>

#include <utils/dijkstramap.hpp>
#include <utils/flowmap.hpp>
#include <dungeon/types.hpp>

DungeonChange::DungeonChange(float collapseCd, float mushroomGenCd, float mushroomNearGenCd):
  collapseCd(collapseCd), mushroomGenCd(mushroomGenCd), mushroomNearGenCd(mushroomNearGenCd),
  collapseTimer(0.f), mushroomGenTimer(mushroomGenCd), mushroomNearGenTimer(mushroomNearGenCd / 2.f)
{
}

void DungeonChange::update(entt::registry& registry, AssetManager& mgr, float dt)
{
  static const auto dungeonView = registry.view<DungeonData>();
  static const auto tileView = registry.view<Tile, TextureSource>();

  collapseTimer += dt;
  mushroomGenTimer += dt;
  mushroomNearGenTimer += dt;

  if (collapseTimer < collapseCd && mushroomGenTimer < mushroomGenCd && mushroomNearGenTimer < mushroomNearGenCd)
    return;
  
  entt::entity dungeonEntity = dungeonView.front();
  DungeonData& dd = registry.get<DungeonData>(dungeonEntity);

  if (collapseTimer >= collapseCd)
  {
    collapseTimer -= collapseCd;

    indexBuffer.clear();
    for (size_t index = 0; index < dd.width * dd.height; ++index)
    {
      if (dd.wallDM[index] >= 4 && dd.tiles[index] == dungeon::floor)
        indexBuffer.push_back(index);
    }

    if (indexBuffer.size() > 0)
    {
      size_t index = indexBuffer[rand() % indexBuffer.size()];
      dd.tiles[index] = dungeon::wall;

      // changeTileTexture(registry, mgr, index, "wall");

      // gen_dijkstra_map(dd.wallDM.data(), dd.tiles.data(), dd.width, dd.height, dungeon::wall);
      // gen_flow_map(dd.wallFM.data(), dd.wallDM.data(), dd.width, dd.height);
    }
  }

  if (mushroomGenTimer >= mushroomGenCd)
  {
    mushroomGenTimer -= mushroomGenCd;

    indexBuffer.clear();
    for (size_t index = 0; index < dd.width * dd.height; ++index)
    {
      if (dd.wallDM[index] >= 2 && dd.tiles[index] == dungeon::floor)
        indexBuffer.push_back(index);
    }

    if (indexBuffer.size() > 0)
    {
      size_t index = indexBuffer[rand() % indexBuffer.size()];
      dd.tiles[index] = dungeon::mushroom;

      // changeTileTexture(registry, mgr, index, "mushroom");

      // gen_dijkstra_map(dd.mushroomDM.data(), dd.tiles.data(), dd.width, dd.height, dungeon::mushroom);
      // gen_flow_map(dd.mushroomFM.data(), dd.mushroomDM.data(), dd.width, dd.height);

      onMushroomGen(registry, mgr, index % dd.width, index / dd.width, dd);
    }
  }
  
  if (mushroomNearGenTimer >= mushroomNearGenCd)
  {
    mushroomNearGenTimer -= mushroomNearGenCd;

    indexBuffer.clear();
    for (size_t index = 0; index < dd.width * dd.height; ++index)
    {
      if (dd.tiles[index] == dungeon::mushroom)
      {
        // Up
        if (index >= dd.width && dd.tiles[index - dd.width] == dungeon::floor)
          indexBuffer.push_back(index - dd.width);
        // Down
        if (index < dd.width * (dd.height - 1) && dd.tiles[index + dd.width] == dungeon::floor)
          indexBuffer.push_back(index + dd.width);
        // Left
        if (index >= 1 && dd.tiles[index - 1] == dungeon::floor)
          indexBuffer.push_back(index - 1);
        // Right
        if (index < dd.width * dd.height - 1 && dd.tiles[index + 1] == dungeon::floor)
          indexBuffer.push_back(index + 1);
      }
    }

    if (indexBuffer.size() > 0)
    {
      size_t index = indexBuffer[rand() % indexBuffer.size()];
      dd.tiles[index] = dungeon::mushroom;

      // changeTileTexture(registry, mgr, index, "mushroom");

      // gen_dijkstra_map(dd.mushroomDM.data(), dd.tiles.data(), dd.width, dd.height, dungeon::mushroom);
      // gen_flow_map(dd.mushroomFM.data(), dd.mushroomDM.data(), dd.width, dd.height);

      onMushroomGen(registry, mgr, index % dd.width, index / dd.width, dd);
    }
  }

  bool wallsDirty = false;
  bool mushroomDirty = false;
  bool mushroomTreeDirty = false;

  // Sync tiles with dungeon data
  for (auto&& [entity, tile, texSrc]: tileView.each())
  {
    if (dd.tiles[tile.index] == tile.value)
      continue;
    
    dungeon::DungeonTile changedTile = (dd.tiles[tile.index] != dungeon::floor) ?
      (dungeon::DungeonTile) dd.tiles[tile.index] : tile.value;
    
    if (changedTile == dungeon::wall)
    {
      wallsDirty = true;
      mushroomDirty = true;
      mushroomTreeDirty = true;
    }
    else if (changedTile == dungeon::mushroom)
    {
      mushroomDirty = true;
      
      texSrc.tex = mgr.getTexture("mushroom");
    }
    else if (changedTile == dungeon::mushroomTree)
    {
      mushroomTreeDirty = true;
      
      texSrc.tex = mgr.getTexture("mushroomTree");
    }

    tile.value = (dungeon::DungeonTile) dd.tiles[tile.index];

    if      (dd.tiles[tile.index] == dungeon::floor)
      texSrc.tex = mgr.getTexture("floor");
    else if (dd.tiles[tile.index] == dungeon::wall)
      texSrc.tex = mgr.getTexture("wall");
    else if (dd.tiles[tile.index] == dungeon::mushroom)
      texSrc.tex = mgr.getTexture("mushroom");
    else if (dd.tiles[tile.index] == dungeon::mushroomTree)
      texSrc.tex = mgr.getTexture("mushroomTree");

    if (wallsDirty)
    {
      gen_dijkstra_map(dd.wallDM.data(), dd.tiles.data(), dd.width, dd.height, dungeon::wall);
      gen_flow_map(dd.wallFM.data(), dd.wallDM.data(), dd.width, dd.height);
    }

    if (mushroomDirty)
    {
      gen_dijkstra_map(dd.mushroomDM.data(), dd.tiles.data(), dd.width, dd.height, dungeon::mushroom);
      gen_flow_map(dd.mushroomFM.data(), dd.mushroomDM.data(), dd.width, dd.height);
    }

    if (mushroomTreeDirty)
    {
      gen_dijkstra_map(dd.mushroomTreeDM.data(), dd.tiles.data(), dd.width, dd.height, dungeon::mushroomTree);
      gen_flow_map(dd.mushroomTreeFM.data(), dd.mushroomTreeDM.data(), dd.width, dd.height);
    }
  }
}

void DungeonChange::changeTileTexture(entt::registry& registry, AssetManager& mgr, size_t index, std::string texName)
{
  static auto tileView = registry.view<Tile, TextureSource>();

  for (auto&& [entity, tile, texSrc]: tileView.each())
  {
    if (tile.index == index)
    {
      texSrc.tex = mgr.getTexture(texName);
      return;
    }
  }
}

void DungeonChange::onMushroomGen(entt::registry& registry, AssetManager& mgr, long x, long y, DungeonData& dd)
{
  constexpr long dx[] = {0, -1, 1,  0, 0};
  constexpr long dy[] = {0,  0, 0, -1, 1};

  for (int i = 0; i < 5; ++i)
  {
    // Center
    long xx = x + dx[i];
    long yy = y + dy[i];

    if (xx <= 0 || xx >= dd.width - 1 || y <= 0 || y >= dd.height - 1)
      continue;
    
    bool isMushroomTree = true;
    for (int j = 0; (j < 5) && isMushroomTree; ++j)
    {
      isMushroomTree = (dd.tiles[(yy + dy[j]) * dd.width + (xx + dx[j])] == dungeon::mushroom);
    }

    if (isMushroomTree)
    {
      size_t index;
      for (int j = 1; j < 5; ++j)
      {
        index = (yy + dy[j]) * dd.width + (xx + dx[j]);
        dd.tiles[index] = dungeon::floor;
        // changeTileTexture(registry, mgr, index, "floor");
      }

      index = yy * dd.width + xx;
      dd.tiles[index] = dungeon::mushroomTree;
      // changeTileTexture(registry, mgr, index, "mushroomTree");

      // gen_dijkstra_map(dd.mushroomDM.data(), dd.tiles.data(), dd.width, dd.height, dungeon::mushroom);
      // gen_dijkstra_map(dd.mushroomTreeDM.data(), dd.tiles.data(), dd.width, dd.height, dungeon::mushroomTree);
      // gen_flow_map(dd.mushroomFM.data(), dd.mushroomDM.data(), dd.width, dd.height);
      // gen_flow_map(dd.mushroomTreeFM.data(), dd.mushroomTreeDM.data(), dd.width, dd.height);
      return;
    }
  }
}
