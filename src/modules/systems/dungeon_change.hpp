#pragma once

#include "systems_manager.hpp"

#include <ecs/types.hpp>

class DungeonChange : public System
{
public:
  // Cd is countdown
  DungeonChange(float collapseCd, float mushroomGenCd, float mushroomNearGenCd);

  void update(entt::registry& registry, AssetManager& mgr, float dt) override;

private:
  float collapseCd;
  float mushroomGenCd;
  float mushroomNearGenCd;

  float collapseTimer = 0.f;
  float mushroomGenTimer = 0.f;
  float mushroomNearGenTimer = 0.f;

  std::vector<size_t> indexBuffer;

  void changeTileTexture(entt::registry& registry, AssetManager& mgr, size_t index, std::string texName);
  void onMushroomGen(entt::registry& registry, AssetManager& mgr, long x, long y, DungeonData& dd);
};
