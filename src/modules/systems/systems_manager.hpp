#pragma once

#include <entt\entt.hpp>
#include <vector>

#include <asset_manager/asset_manager.hpp>

class System
{
public:
  virtual void update(entt::registry& registry, AssetManager& mgr, float dt) = 0;
};

class SystemsManager
{
public:
  SystemsManager(entt::registry& registry, AssetManager& mgr);

  void register_system(System* system);
  void update(float dt);

private:
  entt::registry& registry;
  AssetManager& mgr;
  std::vector<System*> systems;
};
