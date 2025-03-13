#include "utils.hpp"
#include "types.hpp"

#include <raylib.h>
#include <raymath.h>

#include <utils/dijkstramap.hpp>
#include <utils/flowmap.hpp>

Position dungeon::find_walkable_tile(entt::registry& registry)
{
  static auto dungeonDataView = registry.view<DungeonData>();

  entt::entity e = dungeonDataView.front();
  const DungeonData& dd = registry.get<const DungeonData>(e);

  Position res{0, 0};
  // dungeonDataView.each([&](const DungeonData &dd)
  // {
    // prebuild all walkable and get one of them
    std::vector<Position> posList;
    for (size_t y = 0; y < dd.height; ++y)
      for (size_t x = 0; x < dd.width; ++x)
        if (dd.tiles[y * dd.width + x] == dungeon::floor)
          posList.push_back(Position{float(x), float(y)});
    size_t rndIdx = size_t(GetRandomValue(0, int(posList.size()) - 1));
    res = posList[rndIdx];
  // });
  return res;
}

bool dungeon::is_tile_walkable(entt::registry& registry, Position pos)
{
  static auto dungeonDataView = registry.view<DungeonData>();

  entt::entity e = dungeonDataView.front();
  const DungeonData& dd = registry.get<const DungeonData>(e);

  bool res = false;
  // dungeonDataQuery.each([&](const DungeonData &dd)
  // {
    if (pos.x < 0 || pos.x >= int(dd.width) ||
        pos.y < 0 || pos.y >= int(dd.height))
      return res;
    res = dd.tiles[size_t(pos.y) * dd.width + size_t(pos.x)] == dungeon::floor;
  // });
  return res;
}

void dungeon::init_dungeon(entt::registry& registry, AssetManager& mgr, char *tiles, size_t w, size_t h)
{
  std::vector<char> dungeonData;
  std::vector<char> wallsDM;
  std::vector<char> mushroomDM;
  std::vector<char> mushroomTreeDM;
  std::vector<Vector2> wallsFM;
  std::vector<Vector2> mushroomFM;
  std::vector<Vector2> mushroomTreeFM;

  dungeonData.resize(w * h);
  wallsDM.resize(w * h);
  mushroomDM.resize(w * h);
  mushroomTreeDM.resize(w * h);
  std::memcpy(dungeonData.data(), tiles, w * h * sizeof(char));
  
  gen_dijkstra_map(wallsDM.data(), tiles, w, h, dungeon::wall);
  gen_dijkstra_map(mushroomDM.data(), tiles, w, h, dungeon::mushroom);
  gen_dijkstra_map(mushroomTreeDM.data(), tiles, w, h, dungeon::mushroomTree);
  
  wallsFM.resize(w * h);
  mushroomFM.resize(w * h);
  mushroomTreeFM.resize(w * h);

  gen_flow_map(wallsFM.data(), wallsDM.data(), w, h);
  gen_flow_map(mushroomFM.data(), mushroomDM.data(), w, h);
  gen_flow_map(mushroomTreeFM.data(), mushroomTreeDM.data(), w, h);

  for (size_t x = 1; x < w - 1; ++x)
  {
    // just 'x' actually, but for consitency
    dungeonData[0 * w + x] = dungeon::border;
    wallsFM[0 * w + x] = Vector2(0, 1);
    mushroomFM[0 * w + x] = Vector2(0, 1);
    mushroomTreeFM[0 * w + x] = Vector2(0, 1);

    dungeonData[(h - 1) * w + x] = dungeon::border;
    wallsFM[(h - 1) * w + x] = Vector2(0, -1);
    mushroomFM[(h - 1) * w + x] = Vector2(0, -1);
    mushroomTreeFM[(h - 1) * w + x] = Vector2(0, -1);
  }

  for (size_t y = 1; y < h - 1; ++y)
  {
    // just 'y * w' actually, but for consitency
    dungeonData[y * w + 0] = dungeon::border;
    wallsFM[y * w + 0] = Vector2(1, 0);
    mushroomFM[y * w + 0] = Vector2(1, 0);
    mushroomTreeFM[y * w + 0] = Vector2(1, 0);

    dungeonData[y * w + (w - 1)] = dungeon::border;
    wallsFM[y * w + (w - 1)] = Vector2(-1, 0);
    mushroomFM[y * w + (w - 1)] = Vector2(-1, 0);
    mushroomTreeFM[y * w + (w - 1)] = Vector2(-1, 0);
  }

  // Top left
  dungeonData[0 * w + 0] = dungeon::border;
  wallsFM[0 * w + 0] = Vector2Normalize({ 1, 1 });
  mushroomFM[0 * w + 0] = Vector2Normalize({ 1, 1 });
  mushroomTreeFM[0 * w + 0] = Vector2Normalize({ 1, 1 });

  // Top right
  dungeonData[0 * w + (w - 1)] = dungeon::border;
  wallsFM[0 * w + (w - 1)] = Vector2Normalize({ -1, 1 });
  mushroomFM[0 * w + (w - 1)] = Vector2Normalize({ -1, 1 });
  mushroomTreeFM[0 * w + (w - 1)] = Vector2Normalize({ -1, 1 });
  
  // Down left
  dungeonData[(h - 1) * w + 0] = dungeon::border;
  wallsFM[(h - 1) * w + 0] = Vector2Normalize({ 1, -1 });
  mushroomFM[(h - 1) * w + 0] = Vector2Normalize({ 1, -1 });
  mushroomTreeFM[(h - 1) * w + 0] = Vector2Normalize({ 1, -1 });
  
  // Down right
  dungeonData[(h - 1) * w + (w - 1)] = dungeon::border;
  wallsFM[(h - 1) * w + (w - 1)] = Vector2Normalize({ -1, -1 });
  mushroomFM[(h - 1) * w + (w - 1)] = Vector2Normalize({ -1, -1 });
  mushroomTreeFM[(h - 1) * w + (w - 1)] = Vector2Normalize({ -1, -1 });

  entt::entity dungeon = registry.create();

  registry.emplace<DungeonData>(
    dungeon,
    DungeonData{
      std::move(dungeonData),
      std::move(wallsDM),
      std::move(mushroomDM),
      std::move(mushroomTreeDM),
      std::move(wallsFM),
      std::move(mushroomFM),
      std::move(mushroomTreeFM),
      w,
      h
    }
  );

  constexpr float tile_size = 64.f;

  std::printf("Here!\n");

  for (size_t y = 0; y < h; ++y)
    for (size_t x = 0; x < w; ++x)
    {
      const size_t idx = y * w + x;
      char tile = tiles[idx];
      entt::entity tileEntity = registry.create();
      registry.emplace<Tile>(tileEntity, Tile{ idx, (dungeon::DungeonTile) tiles[idx] });
      registry.emplace<Position>(tileEntity, Position{float(x) * tile_size, float(y) * tile_size});

      if (tile == dungeon::wall || tile == dungeon::border)
        registry.emplace<TextureSource>(tileEntity, TextureSource{mgr.getTexture("wall")});
      else if (tile == dungeon::floor)
        registry.emplace<TextureSource>(tileEntity, TextureSource{mgr.getTexture("floor")});
    }
}

// void dungeon::replaceTile(entt::registry& registry, AssetManager& mgr, dungeon::DungeonTile newTile, size_t index)
// {
//   if (newTile == dungeon::mushroomTree)
//     return;  // Should not be used

//   static auto dungeonView = registry.view<DungeonData>();
//   static auto tileView = registry.view<Tile, TextureSource>();

//   entt::entity ddEntity = dungeonView.front();
//   DungeonData& dd = registry.get<DungeonData>(ddEntity);

//   DungeonTile prevTile = (DungeonTile) dd.tiles[index];
//   DungeonTile changed = prevTile;

//   dd.tiles[index] = newTile;

//   if (changed == dungeon::floor)
//     changed = newTile;  // In case floor was replaced by something

//   for (auto&& [entity, tile, texSrc]: tileView.each())
//   {
//     if (tile.index != index)
//       continue;
    
//     switch (changed)
//     {
//     case dungeon::wall:
//       gen_dijkstra_map(dd.wallDM.data(), dd.tiles.data(), dd.width, dd.height, dungeon::wall);
//       gen_flow_map(dd.wallFM.data(), dd.wallDM.data(), dd.width, dd.height);
//       texSrc.tex = (newTile == dungeon::wall) ? mgr.getTexture("wall") : mgr.getTexture("floor");
//       break;
    
//     case dungeon::mushroom:
//       gen_dijkstra_map(dd.mushroomDM.data(), dd.tiles.data(), dd.width, dd.height, dungeon::mushroom);
//       gen_flow_map(dd.mushroomFM.data(), dd.mushroomDM.data(), dd.width, dd.height);
//       texSrc.tex = (newTile == dungeon::mushroom) ? mgr.getTexture("mushroom") : mgr.getTexture("floor");
//       break;
//     }
//   }
// }
