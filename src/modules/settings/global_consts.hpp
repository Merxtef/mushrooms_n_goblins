#pragma once

namespace Settings
{
  constexpr float fatigueDecay = 0.6f; // [0, 100] fatigue decay per second (non work state)
  constexpr float hungerDecay = 0.8f; // [0, 100] hunger decay per second (non work state)
  constexpr float fatigueWorkMult = 2.f; // 1+ fatigue decay multiplier when working
  constexpr float hungerWorkMult = 2.f; // 1+ hunger decay multiplier when working

  constexpr float restSpeed = 1.f;  // 0.01+ fatigue restoration while resting (not near mushroom tree)
  constexpr float restSpeedTree = 2.f;  // 0.01+ fatigue restoration while resting near mushroom tree

  constexpr float dwarfSpawnChance = 0.025; // [0, 1] chance of dwarf spawn per wall destroyed

  // Seconds until...
  constexpr float collapseCountdown = 8.f; // 1+ ... floor collapses into wall (dist to wall >= 4 tile)
  constexpr float mushroomSpawnCountdown = 5.f; // 1+ ... mushroom spawn (dist to wall >= 2 tile)
  constexpr float mushroomNearSpawnCountdown = 2.f; // ... mushroom spawn near exisiting mushroom (if any)



  // Better not to touch
  constexpr float TILE_SIZE = 64.f;
  constexpr float HALF_TILE_SIZE = TILE_SIZE / 2.f;
  constexpr int DUNGEON_WIDTH = 50;
  constexpr int DUNGEON_HEIGHT = 40;
}
