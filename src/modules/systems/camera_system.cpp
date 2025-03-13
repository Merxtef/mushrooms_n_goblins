#include "camera_system.hpp"

#include <ecs/types.hpp>

void CameraSystem::update(entt::registry& registry, AssetManager& mgr, float dt)
{
  static auto playerView = registry.view<IsPlayer, Position>();
  static auto cameraView = registry.view<Camera2D>();

  entt::entity playerEntity = playerView.front();
  const Position& pos = registry.get<const Position>(playerEntity);

  entt::entity cameraEntity = cameraView.front();
  Camera2D& camera = registry.get<Camera2D>(cameraEntity);

  camera.offset = pos;
}