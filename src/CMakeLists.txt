add_library(modules)

file(GLOB_RECURSE MODULES_SOURCES CONFIGURE_DEPENDS modules/**/*.cpp)
file(GLOB_RECURSE MODULES_HEADERS CONFIGURE_DEPENDS modules/**/*.hpp)

target_sources(modules
  PRIVATE ${MODULES_SOURCES}
  PUBLIC ${MODULES_HEADERS}
)

target_include_directories(modules PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/modules)
target_link_libraries(modules PUBLIC raylib)
target_include_directories(modules PUBLIC ${EnTT_SOURCE_DIR}/single_include)

add_executable(game main.cpp)
target_link_libraries(game PRIVATE modules)
