#include "asset_manager.hpp"

AssetManager::AssetManager()
{
}

void AssetManager::addTexture(const char* path, std::string name)
{
  textures_[name] = LoadTexture(path);
}

Texture2D AssetManager::getTexture(std::string name)
{
  return textures_.at(name);
}
