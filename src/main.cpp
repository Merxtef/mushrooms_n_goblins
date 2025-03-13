#include <raylib.h>
#include <cstdio>
#include <raylib.h>
#include <entt/entt.hpp>

#include <dungeon/generator.hpp>
#include <dungeon/utils.hpp>
#include <utils/dijkstramap.hpp>
#include <asset_manager/asset_manager.hpp>
#include <systems/systems_manager.hpp>
#include <systems/camera_system.hpp>
#include <systems/movement.hpp>
#include <systems/dungeon_change.hpp>
#include <utils/math.hpp>
#include <mobs/dwarf.hpp>
#include <settings/global_consts.hpp>

void print_grid(char* grid, size_t w, size_t h)
{
  for (size_t y = 0; y < h; ++y)
    std::printf("%.*s\n", int(w), grid + y * w);
}

int main()
{
  constexpr int WIDTH = Settings::DUNGEON_WIDTH;
  constexpr int HEIGH = Settings::DUNGEON_HEIGHT;
  char dungeon[WIDTH * HEIGH] = {};
  gen_drunk_dungeon(dungeon, WIDTH, HEIGH);

  constexpr int WINDOW_WIDTH = 2000;
  constexpr int WINDOW_HEIGHT = 1000;
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Test");


  entt::registry registry;
  registry.group<Tile>();
  
  AssetManager mgr;
  mgr.addTexture("assets/wall.png", "wall");
  mgr.addTexture("assets/floor.png", "floor");
  mgr.addTexture("assets/mushroom.png", "mushroom");
  mgr.addTexture("assets/mushroomTree.png", "mushroomTree");
  mgr.addTexture("assets/arrow.png", "arrow");

  dungeon::init_dungeon(registry, mgr, dungeon, WIDTH, HEIGH);

  Camera2D camera = {
    .offset = {400, 300},
    .target = {400 - 1, 300 - 1},
    .rotation = 0.f,
    .zoom = 1.f
  };

  entt::entity cameraEntity = registry.create();
  registry.emplace<Camera2D>(cameraEntity, camera);

  entt::entity playerEntity = registry.create();
  registry.emplace<IsPlayer>(playerEntity);
  registry.emplace<Position>(playerEntity, Position{400, 300});

  SystemsManager sysMgr(registry, mgr);
  sysMgr.register_system(new PlayerMovement(180.f, 320.f)); // Since "player" is not part of the world - that will suffice
  sysMgr.register_system(new CameraSystem());
  sysMgr.register_system(new DwarfSystem());
  sysMgr.register_system(new DungeonChange(Settings::collapseCountdown, Settings::mushroomSpawnCountdown, Settings::mushroomNearSpawnCountdown));
  sysMgr.register_system(new Steering());
  sysMgr.register_system(new EntityMovement());

  auto drawableView = registry.view<Position, TextureSource>();
  auto tileView = registry.view<Position, Tile>();
  auto dungeonView = registry.view<DungeonData>();
  auto dwarfView = registry.view<Dwarf, Position, Velocity, SteerDir>();

  bool drawDwarfStat = false;
  bool drawDijkstra = false;
  bool drawFlow = false;
  bool drawHelp = true;

  float dijkstra_opacity = 50.f;
  int map_num = 0;

  {
    // Finding free unique spaces for dwarfs
    // Not optimal, but will suffice
    constexpr int DWARF_COUNT = 5;
    size_t indexes[DWARF_COUNT] = {};

    std::vector<size_t> tileIndexes;

    for (size_t y = 1; y < HEIGH - 1; ++y)
      for (size_t x = 1; x < WIDTH -1; ++x)
      {
        if (dungeon[y * WIDTH + x] == dungeon::floor)
        {
          tileIndexes.push_back(y * WIDTH + x);
        }
      }
    
    for (int i = 0; i < DWARF_COUNT; ++i)
    {
      indexes[i] = rand() % tileIndexes.size();
    }

    bool flag = true;

    while (flag)
    {
      flag = false;
      for (int i = 0; i < DWARF_COUNT; ++i)
        for (int j = i + 1; j < DWARF_COUNT; ++j)
        {
          if (indexes[i] == indexes[j])
          {
            indexes[j] = rand() % tileIndexes.size();
            flag = true;
          }
        }
    }

    for (int i = 0; i < DWARF_COUNT; ++i)
    {
      size_t tileIndex = tileIndexes[indexes[i]];
      create_dwarf(registry, getCellCenter({ (int) (tileIndex % WIDTH), (int) (tileIndex / WIDTH) }));
    }
  }


  Texture2D arrow = mgr.getTexture("arrow");

  SetTargetFPS(60);
  while (!WindowShouldClose())
  {
    float dt = GetFrameTime();
    if (dt > 0.5f)
      dt = 0.5f;
    sysMgr.update(dt);
    camera = registry.get<Camera2D>(cameraEntity);

    auto dungeonEntity = dungeonView.front();
    const auto& dd = registry.get<const DungeonData>(dungeonEntity);

    if (IsKeyPressed(KEY_Z))
      drawDwarfStat = !drawDwarfStat;
    if (IsKeyPressed(KEY_X))
      drawDijkstra = !drawDijkstra;
    if (IsKeyPressed(KEY_C))
      drawFlow = !drawFlow;
    if (IsKeyPressed(KEY_H))
      drawHelp = !drawHelp;

    if (IsKeyPressed(KEY_ONE))
      map_num = 0;
    if (IsKeyPressed(KEY_TWO))
      map_num = 1;
    if (IsKeyPressed(KEY_THREE))
      map_num = 2;

    BeginDrawing();
      ClearBackground(BLACK);
      BeginMode2D(camera);

      for (auto&& [entity, pos, texSrc] : drawableView.each())
      {
        DrawTexturePro(
          texSrc.tex,
          {0, 0, 512, 512},
          {pos.x, pos.y, Settings::TILE_SIZE, Settings::TILE_SIZE},
          {0, 0},
          0.f,
          WHITE
        );
      }

      for (auto&& [entity, dwarf, pos, v, sd]: dwarfView.each())
      {
        DrawCircle(pos.x, pos.y, 16.f, RED);

        if (drawDwarfStat)
        {
          DrawLine(pos.x, pos.y, pos.x + v.vector.x, pos.y + v.vector.y, GREEN);

          char buf[256];
          switch (dwarf.state)
          {
          case WallSearch:
            sprintf_s(buf, "State: WallSearch");
            break;
          case MushroomSearch:
            sprintf_s(buf, "State: MushroomSearch");
            break;
          case MushroomTreeSearch:
            sprintf_s(buf, "State: TreeSearch");
            break;
          case WallDigging:
            sprintf_s(buf, "State: WallDigging");
            break;
          case Resting:
            sprintf_s(buf, "State: Resting");
            break;
          }

          IVec2 boxPos = { pos.x + 20, pos.y - 120 };

          DrawRectangle(boxPos.x, boxPos.y, 200, 80, { 0, 0, 0, 120 });
          DrawText(buf, boxPos.x + 10, boxPos.y + 10, 16, { 255, 255, 255, 120 });

          DrawText("Hunger:", boxPos.x + 10, boxPos.y + 28, 16, { 255, 255, 255, 120 });
          DrawRectangle(boxPos.x + 80, boxPos.y + 28, 100, 16, { 188, 188, 188, 80 });
          if (dwarf.hunger > 0.5f)
          {
            DrawRectangle(boxPos.x + 80, boxPos.y + 28, dwarf.hunger, 16, { 255, 188, 0, 80 });
          }

          DrawText("Fatigue:", boxPos.x + 10, boxPos.y + 46, 16, { 255, 255, 255, 120 });
          DrawRectangle(boxPos.x + 80, boxPos.y + 46, 100, 16, { 188, 188, 188, 80 });
          if (dwarf.hunger > 0.5f)
          {
            DrawRectangle(boxPos.x + 80, boxPos.y + 46, dwarf.fatigue, 16, { 0, 188, 0, 80 });
          }
        }
      }

      if (drawDijkstra || drawFlow)
        for (auto&& [entity, pos, tile]: tileView.each())
        {
          int x = tile.index % WIDTH;
          int y = tile.index / WIDTH;

          if (drawDijkstra)
          {
            char heatValue = dd.wallDM[tile.index];
            if (map_num == 1)
              heatValue = dd.mushroomDM[tile.index];
            else if (map_num == 2)
              heatValue = dd.mushroomTreeDM[tile.index];

            Vector3 heatColor = getHeatmapColor(heatValue);

            dijkstra_opacity = Clamp(dijkstra_opacity + GetMouseWheelMove() * 0.01f, 0.f, 255.f);

            DrawRectangle(
              pos.x, pos.y,
              (int) Settings::TILE_SIZE, (int) Settings::TILE_SIZE,
              Color{
                (unsigned char) heatColor.x,
                (unsigned char) heatColor.y,
                (unsigned char) heatColor.z,
                (unsigned char) dijkstra_opacity
              }
            );
          }
          
          if (drawFlow)
          {
            Vector2 flowValue = dd.wallFM[tile.index];
            if (map_num == 1)
              flowValue = dd.mushroomFM[tile.index];
            else if (map_num == 2)
              flowValue = dd.mushroomTreeFM[tile.index];

            if (Vector2Length(flowValue) > 0.5f)
              DrawTexturePro(
                arrow,
                {0, 0, 560, 560},
                {pos.x + Settings::HALF_TILE_SIZE, pos.y + Settings::HALF_TILE_SIZE, Settings::TILE_SIZE, Settings::TILE_SIZE},
                {Settings::HALF_TILE_SIZE, Settings::HALF_TILE_SIZE},
                Vector2Angle({ 1.f, 0.f }, flowValue) * (180.f / PI),
                WHITE
              );
          }
        }

      EndMode2D();

      if (drawHelp)
      {
        int width = 340;
        int height = 334;
        IVec2 boxPos = { WINDOW_WIDTH - width, 0 };
        DrawRectangle(boxPos.x, boxPos.y, width, height, { 0, 0, 0, 120 });

        Color fontColor = { 255, 255, 255, 160 };

        DrawText("[WASD] - Camera movement", boxPos.x + 10, boxPos.y + 10, 24, fontColor);
        DrawText("[Z] - Dwarf stats", boxPos.x + 10, boxPos.y + 40, 24, fontColor);
        DrawText("[X] - Dijkstra Map", boxPos.x + 10, boxPos.y + 70, 24, fontColor);
        DrawText("[C] - Flow Map", boxPos.x + 10, boxPos.y + 100, 24, fontColor);
        DrawText("[Scroll] - Dijstra Opacity", boxPos.x + 10, boxPos.y + 130, 24, fontColor);

        DrawText("Dijstra/Flow map types:", boxPos.x + 10, boxPos.y + 170, 24, fontColor);
        DrawText("[1] - Wall map", boxPos.x + 10, boxPos.y + 200, 24, fontColor);
        DrawText("[2] - Mushroom map", boxPos.x + 10, boxPos.y + 230, 24, fontColor);
        DrawText("[3] - Tree map", boxPos.x + 10, boxPos.y + 260, 24, fontColor);

        DrawText("[H] - Close/Open this menu", boxPos.x + 10, boxPos.y + 300, 24, fontColor);
      }
    EndDrawing();
  }
  
}
