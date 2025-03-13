#pragma once

#include <raylib.h>
#include <unordered_map>

class AssetManager
{
public:
  AssetManager();

  void addTexture(const char* path, std::string name);
  Texture2D getTexture(std::string name);

private:
  std::unordered_map<std::string, Texture2D> textures_;
};
