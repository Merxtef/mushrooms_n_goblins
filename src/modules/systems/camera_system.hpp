#pragma once

#include "systems_manager.hpp"

class CameraSystem : public System
{
public:
  void update(entt::registry& registry, AssetManager& mgr, float dt) override;
};
