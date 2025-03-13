#pragma once

#include <entt/entt.hpp>

#include <ecs/types.hpp>
#include <asset_manager/asset_manager.hpp>
#include "types.hpp"

namespace dungeon
{
  Position find_walkable_tile(entt::registry& registry);
  bool is_tile_walkable(entt::registry& registry, Position pos);
  void init_dungeon(entt::registry& registry, AssetManager& mgr, char *tiles, size_t w, size_t h);
  void replaceTile(entt::registry& registry, AssetManager& mgr, DungeonTile newTile, size_t index);
};