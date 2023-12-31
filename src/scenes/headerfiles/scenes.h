#pragma once

#include "../../core/headerfiles/header.h"

#include "start_scene.h"
#include "main_scene.h"

void GenerateScenes(std::vector<std::shared_ptr<Scene>>* scenes) {
	std::shared_ptr<StartScene> startScene = std::make_shared<StartScene>();
	scenes->push_back(startScene);

	std::shared_ptr<MainScene> mainMenuScene = std::make_shared<MainScene>();
	scenes->push_back(mainMenuScene);
}