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

# SDL3版フロントエンドの依存ライブラリ
find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(SDL3_ttf REQUIRED)
find_package(SDL3_mixer REQUIRED)

# Luaスクリプティング（リソース定義等）用の依存ライブラリ
find_package(Lua REQUIRED)
if(NOT TARGET Lua::Lua)
  add_library(Lua::Lua INTERFACE IMPORTED)
  set_target_properties(Lua::Lua PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${LUA_INCLUDE_DIR}"
    INTERFACE_LINK_LIBRARIES "${LUA_LIBRARIES}"
  )
endif()
find_package(sol2 REQUIRED)

add_subdirectory(src)
