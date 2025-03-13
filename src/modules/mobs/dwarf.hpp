#pragma once

#include <entt/entt.hpp>

#include <ecs/types.hpp>
#include <systems/systems_manager.hpp>
#include <settings/global_consts.hpp>

void create_dwarf(entt::registry& registry, Position position);

class DwarfSystem : public System
{
public:
  void update(entt::registry& registry, AssetManager& mgr, float dt) override;

private:
  SteerDir dirAccordingFlow(const std::vector<Vector2>& flowMap, const DungeonData& dd, Position position);
};
