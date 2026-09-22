# SDL3版アプリケーションを構成するモジュール群
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

# SDL3本体はほぼ全モジュールが直接・間接に利用する
target_link_libraries(app       PUBLIC SDL3::SDL3)
target_link_libraries(resources PUBLIC SDL3::SDL3)
target_link_libraries(scene     PUBLIC SDL3::SDL3)
target_link_libraries(sdl       PUBLIC SDL3::SDL3)

# 画像・フォント・音声を直接扱うモジュールのみ SDL3_image / SDL3_ttf / SDL3_mixer を追加リンク
target_link_libraries(app PUBLIC SDL3_image::SDL3_image SDL3_ttf::SDL3_ttf SDL3_mixer::SDL3_mixer)
target_link_libraries(sdl PUBLIC SDL3_image::SDL3_image SDL3_ttf::SDL3_ttf SDL3_mixer::SDL3_mixer)

# リソース定義（文字列・画像テーブル等）およびMenuSceneの描画レイアウトを
# Luaスクリプトから読み込むための依存
target_link_libraries(resources PRIVATE Lua::Lua sol2::sol2)
target_link_libraries(scene     PRIVATE Lua::Lua sol2::sol2)

# 移植済みゲームロジック本体（C -> C++、プラットフォーム非依存部分）
# main.c / 旧 src/win32, src/x11 の画面・入力・音声実装層は未移植
set(XANADU_CORE_SOURCES
  animation.cpp
  battle.cpp
  battle/BattleState.cpp
  boss.cpp
  cave.cpp
  context.cpp
  dungeon.cpp
  equip.cpp
  fade.cpp
  field.cpp
  goods.cpp
  graphics.cpp
  inventory.cpp
  message.cpp
  numinous.cpp
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

# context.cpp が Application / GameScene派生クラス群を介して
# 画面遷移をSDL3側のシーン機構に橋渡ししているための依存
target_link_libraries(xanadu_core PUBLIC app scene)

# SDL3ベースの新フロントエンド実行ファイル
add_executable(xanadu main.cpp)

target_link_libraries(xanadu PRIVATE ${XANADU_SDL_MODULES} xanadu_core)
