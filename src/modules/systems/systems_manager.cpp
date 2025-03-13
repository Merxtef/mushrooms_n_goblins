#include "systems_manager.hpp"

SystemsManager::SystemsManager(entt::registry& registry, AssetManager& mgr) :
  registry(registry), mgr(mgr)
{
}

void SystemsManager::register_system(System* system)
{
  systems.push_back(system);
}

void SystemsManager::update(float dt)
{
  for (auto system: systems)
  {
    system->update(registry, mgr, dt);
  }
}
