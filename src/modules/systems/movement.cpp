#include "movement.hpp"

#include <raymath.h>

#include <ecs/types.hpp>

PlayerMovement::PlayerMovement(float speed, float shiftSpeed):
  speed(speed), shiftSpeed(shiftSpeed)
{
}

void PlayerMovement::update(entt::registry& registry, AssetManager& mgr, float dt)
{
  Vector2 movement = {
    IsKeyDown(KEY_A) - IsKeyDown(KEY_D),
    IsKeyDown(KEY_W) - IsKeyDown(KEY_S)
  };
  movement = Vector2Normalize(movement);

  static auto playerView = registry.view<IsPlayer, Position>();
  entt::entity playerEntity = playerView.front();
  Position& pos = registry.get<Position>(playerEntity);

  float spd = (IsKeyDown(KEY_LEFT_SHIFT)) ? shiftSpeed : speed;
  pos = {Vector2Add(pos, Vector2Scale(movement, dt * spd))};
}

void EntityMovement::update(entt::registry& registry, AssetManager& mgr, float dt)
{
  static const auto movingView = registry.view<Position, const Velocity>();

  for (auto&& [entity, pos, vel]: movingView.each())
  {
    Vector2 newPos = Vector2Add(pos, Vector2Scale(vel.vector, dt));
    pos = { newPos.x, newPos.y };
  }
}

void Steering::update(entt::registry& registry, AssetManager& mgr, float dt)
{
  static const auto steeringView = registry.view<SteerDir>();
  static const auto movingView = registry.view<Velocity, const SteerDir, const SteerAccel>();

  // for (auto&& [entity, sd]: steeringView.each())
  // {
  //   sd = { 0.f, 0.f };
  // }



  for (auto&& [entity, vel, sd, sa]: movingView.each())
  {
    Vector2 steer = Vector2Scale(Vector2ClampValue(sd, 0.f, vel.cap), sa.accel * dt);
    vel.vector = Vector2ClampValue(Vector2Add(vel.vector, steer), 0.f, vel.cap);
  }
}
