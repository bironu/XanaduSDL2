# SDL2版アプリケーションを構成するモジュール群
# （フォルダごとに OBJECT ライブラリとして管理。各フォルダの CMakeLists.txt で
#   file(GLOB) によりそのフォルダの *.cpp を集めている）
add_subdirectory(app)
add_subdirectory(geo)
add_subdirectory(resources)
add_subdirectory(scene)
add_subdirectory(sdl)
add_subdirectory(task)

set(XANADU_SDL_MODULES app geo resources scene sdl task)

foreach(module ${XANADU_SDL_MODULES})
  target_include_directories(${module} PUBLIC ${CMAKE_SOURCE_DIR}/include)
endforeach()

# SDL2本体はほぼ全モジュールが直接・間接に利用する
target_link_libraries(app       PUBLIC SDL2::SDL2)
target_link_libraries(resources PUBLIC SDL2::SDL2)
target_link_libraries(scene     PUBLIC SDL2::SDL2)
target_link_libraries(sdl       PUBLIC SDL2::SDL2)

# 画像・フォント・音声を直接扱うモジュールのみ SDL2_image / SDL2_ttf / SDL2_mixer を追加リンク
target_link_libraries(app PUBLIC SDL2_image::SDL2_image SDL2_ttf::SDL2_ttf SDL2_mixer::SDL2_mixer)
target_link_libraries(sdl PUBLIC SDL2_image::SDL2_image SDL2_ttf::SDL2_ttf SDL2_mixer::SDL2_mixer)

# 移植済みゲームロジック本体（C -> C++、プラットフォーム非依存部分）
# main.c / 旧 src/win32, src/x11 の画面・入力・音声実装層は未移植
set(XANADU_CORE_SOURCES
  animation.cpp
  audio.cpp
  battle.cpp
  boss.cpp
  cave.cpp
  context.cpp
  dungeon.cpp
  ending.cpp
  equip.cpp
  fade.cpp
  field.cpp
  goods.cpp
  graphics.cpp
  image8.cpp
  image16.cpp
  image24.cpp
  image32.cpp
  inventory.cpp
  menu.cpp
  message.cpp
  numinous.cpp
  opening.cpp
  pause.cpp
  shop.cpp
  status.cpp
  tower.cpp
  use_item.cpp
  user.cpp
  user_dead.cpp
  user_io.cpp
)

add_library(xanadu_core STATIC ${XANADU_CORE_SOURCES})

target_include_directories(xanadu_core PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

# SDL2ベースの新フロントエンド実行ファイル
add_executable(xanadu main.cpp)

target_link_libraries(xanadu PRIVATE ${XANADU_SDL_MODULES})
