#pragma once

#include "systems_manager.hpp"

class PlayerMovement : public System
{
public:
  PlayerMovement(float speed, float shiftSpeed);
  void update(entt::registry& registry, AssetManager& mgr, float dt) override;

private:
  float speed;
  float shiftSpeed;
};

class EntityMovement : public System
{
public:
  void update(entt::registry& registry, AssetManager& mgr, float dt) override;
};

class Steering : public System
{
public:
  void update(entt::registry& registry, AssetManager& mgr, float dt) override;
};
