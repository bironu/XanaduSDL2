cmake_minimum_required(VERSION 3.16)
project(xanadu CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# ソースをsrc直下に置いたままの build ディレクトリでの
# in-source ビルドを防止する
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR
    "In-source builds are not allowed. Please build in a separate "
    "directory, e.g.:\n  mkdir build && cd build && cmake .. && cmake --build .")
endif()

# ゲームロジック本体（プラットフォーム非依存部分）
# main.c / src/win32 / src/x11 の画面・入力・音声実装層は今後の移植対象
set(XANADU_CORE_SOURCES
  src/animation.cpp
  src/audio.cpp
  src/battle.cpp
  src/boss.cpp
  src/cave.cpp
  src/context.cpp
  src/dungeon.cpp
  src/ending.cpp
  src/equip.cpp
  src/fade.cpp
  src/field.cpp
  src/goods.cpp
  src/graphics.cpp
  src/image8.cpp
  src/image16.cpp
  src/image24.cpp
  src/image32.cpp
  src/inventory.cpp
  src/menu.cpp
  src/message.cpp
  src/numinous.cpp
  src/opening.cpp
  src/pause.cpp
  src/shop.cpp
  src/status.cpp
  src/tower.cpp
  src/use_item.cpp
  src/user.cpp
  src/user_dead.cpp
  src/user_io.cpp
)

add_library(xanadu_core STATIC ${XANADU_CORE_SOURCES})

target_include_directories(xanadu_core PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)
