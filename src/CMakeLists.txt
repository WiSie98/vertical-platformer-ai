﻿cmake_minimum_required(VERSION 3.11)

include(../cmake/raylib.cmake)

project(game VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(${PROJECT_NAME} main.cpp "core/headerfiles/actor_base.h" "core/actor_base.cpp" "core/headerfiles/camera.h" "core/headerfiles/scene.h" "core/headerfiles/scene_manager.h" "core/camera.cpp" "core/scene.cpp" "core/scene_manager.cpp" "scenes/headerfiles/scenes.h" "scenes/headerfiles/start_scene.h" "scenes/headerfiles/main_scene.h" "scenes/start_scene.cpp" "scenes/main_scene.cpp" "actors/headerfiles/actor_ai.h" "actors/actor_ai.cpp" "actors/headerfiles/actor_player.h" "actors/actor_player.cpp")

# Add config.h.in file
configure_file(config.h.in config.h)
target_include_directories(${PROJECT_NAME} PUBLIC
        "${PROJECT_BINARY_DIR}"
        )

#  Link to Raylib
target_link_libraries(${PROJECT_NAME} raylib)

# Copy assets to assets path
add_custom_target(copy_assets
        COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_LIST_DIR}/../cmake/copy-assets.cmake"
        )
add_dependencies(${PROJECT_NAME} copy_assets)

if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif ()

# CPack instructions
# call e.g. cpack -G ZIP -C Debug to create a binary distribution
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "${PROJECT_NAME}_Bundle")
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/assets" DESTINATION "${PROJECT_NAME}_Bundle")