#include "headerfiles/main_scene.h"

MainScene::MainScene() {
	setSceneType(MAIN_SCENE);

	std::ifstream tileset_description_file("./assets/levels/vp_ts_metroidlevel.json");
	nlohmann::json tileset_description = nlohmann::json::parse(tileset_description_file);
	tileset_description_file.close();

	std::ifstream level_map_file("./assets/levels/vp_lv_metroidlevel.json");
	nlohmann::json level_map = nlohmann::json::parse(level_map_file);
	level_map_file.close();

	this->tile_atlas_texture = LoadTexture(("./assets/graphics/tilesets/" + tileset_description["image"].get<std::string>()).c_str());

	parseLevelBackgroundTiles(tileset_description, level_map);
	//parseLevelForegroundTiles(tileset_description, level_map);
	parseLevelCollider(tileset_description, level_map);
}

MainScene::~MainScene() {}

void MainScene::update(ActorPlayer& player, std::vector<ActorAI>& ai_vector, MainCamera& camera) {
	Vector2 vec = {STARTPOSITION.x, 0};
	this->temp_fitness = 0;

	if (this->generation_time > TIMEPERGENERATION || IsKeyPressed(KEY_SPACE)) {
		geneticAlgorithm(ai_vector);
		this->generation_time = 0;
		this->generation_counter++;
	} else {
		this->generation_time++;
	}

	if (STARTASPLAYER) {
		player.update();
		detectPlayerCollision(player, camera);
		camera.setTarget(player.getCurrentPosition());
	} else {
		for (int i = 0; i <= ai_vector.size() - 1; i++) {
			if (this->temp_fitness < ai_vector[i].getFitness()) {
				this->temp_fitness = ai_vector[i].getFitness();
				vec.y = ai_vector[i].getCurrentPosition().y;
			}
		}
		camera.setTarget(vec);
	}

	for (int i = 0; i < ai_vector.size() - 1; i++) {

		Vector2 platform_distance = {0, 0};
		Vector2 edge_left = {0, 0};
		Vector2 edge_right = {0, 0};
		Rectangle collider_rec = { 0, 0, 16, 16 };

		//Finding the nearest platform edge
		if (ai_vector[i].getIsStanding()) {
			for (int j = 0; j < collider_vector.size(); j++) {

				Rectangle ai_rec = { ai_vector[i].getCurrentPosition().x, ai_vector[i].getCurrentPosition().y, 16, 32 };
				collider_rec.x = collider_vector[j].get()->collider_position.x;
				collider_rec.y = collider_vector[j].get()->collider_position.y;

				if (collider_vector[j].get()->id == 1) {
					if (ai_vector[i].getCurrentPosition().x - 128 < collider_rec.x && collider_rec.x < ai_vector[i].getCurrentPosition().x + 128 && ai_vector[i].getCurrentPosition().y + 128 < collider_rec.y) {
						if (ai_vector[i].getCurrentPosition().x - collider_rec.x < edge_left.x && !(ai_vector[i].getCurrentPosition().x - collider_rec.x < 0)) {
							edge_left.x = collider_rec.x;
							edge_left.y = collider_rec.y;

							Output output = findRightEdge(j + 1, edge_left.x);
							j = output.iterator;
							edge_right.x = output.output;
							edge_right.y = collider_rec.y;

							if (ai_vector[i].getCurrentPosition().x - edge_left.x < edge_right.x - ai_vector[i].getCurrentPosition().x) {
								ai_vector[i].setNearestPlatformEdge(edge_left.x);
							} else {
								ai_vector[i].setNearestPlatformEdge(edge_right.x);
							}
						}
					}
				}
			}
		} else {
			ai_vector[i].setNearestPlatformEdge(256.0f);
		}
		
		//Finding the nearest platform
		for (int j = 0; j < collider_vector.size(); j++) {
			Rectangle ai_rec = { ai_vector[i].getCurrentPosition().x, ai_vector[i].getCurrentPosition().y, 16, 32 };
			collider_rec.x = collider_vector[j].get()->collider_position.x;
			collider_rec.y = collider_vector[j].get()->collider_position.y;

			if (collider_vector[j].get()->id == 1) {
				if (ai_vector[i].getCurrentPosition().x - 128 < collider_rec.x && collider_rec.x < ai_vector[i].getCurrentPosition().x + 128 && ai_vector[i].getCurrentPosition().y - 128 < collider_rec.y && ai_vector[i].getCurrentPosition().y + 16 < collider_rec.y) {
					if (collider_rec.x < ai_vector[i].getCurrentPosition().x) {
						if (platform_distance.x > ai_vector[i].getCurrentPosition().x - collider_rec.x) { // findRightEdge() benutzen
							platform_distance.x = ai_vector[i].getCurrentPosition().x - collider_rec.x;
							platform_distance.y = ai_vector[i].getCurrentPosition().y - collider_rec.y;
						}
					} else {
						if (platform_distance.x > collider_rec.x - ai_vector[i].getCurrentPosition().x) {
							platform_distance.x = collider_rec.x - ai_vector[i].getCurrentPosition().x;
							platform_distance.y = ai_vector[i].getCurrentPosition().y - collider_rec.y;
						}
					}
				}
			}
		}

		ai_vector[i].setNearestPlatformDistance(platform_distance.x + platform_distance.y);

		platform_distance.x = 0;
		platform_distance.y = 0;

		//Finding the nearest platform beneath distance
		for (int j = 0; j < collider_vector.size(); j++) {
			Rectangle ai_rec = { ai_vector[i].getCurrentPosition().x, ai_vector[i].getCurrentPosition().y, 16, 32 };
			collider_rec.x = collider_vector[j].get()->collider_position.x;
			collider_rec.y = collider_vector[j].get()->collider_position.y;

			if (collider_vector[j].get()->id == 1) {
				if (ai_vector[i].getCurrentPosition().x == collider_rec.x && ai_vector[i].getCurrentPosition().y - 32 < collider_rec.y && ai_vector[i].getCurrentPosition().y + 256 < collider_rec.y) {
					if (platform_distance.y - ai_vector[i].getCurrentPosition().y < platform_distance.y) {
						platform_distance.y = platform_distance.y - ai_vector[i].getCurrentPosition().y;
					}
				}
			}
		}
		ai_vector[i].setNearestPlatformBeneathDistance(platform_distance.y);
		//ai_vector[i].setNearestPlatformBeneathDistance(GetRandomValue(-1024, 1024));

		ai_vector[i].update();
		detectAICollision(ai_vector[i]);
	}

}

void MainScene::draw(ActorPlayer& player, std::vector<ActorAI>& ai_vector, MainCamera& camera) {
	BeginMode2D(camera.getMainCamera());
	drawBackground(camera);
	if (player.getIsActive()) {
		player.draw();
	}

	for (int i = 0; i < ai_vector.size() - 1; i++) {
		ai_vector[i].draw();
	}
	DrawFPS(camera.getMainCamera().target.x - 470, camera.getMainCamera().target.y - 260);
	DrawText(TextFormat("Generation: %i", this->generation_counter), camera.getMainCamera().target.x - 470, camera.getMainCamera().target.y - 235, 20, WHITE);
	DrawText(TextFormat("Highest Fitness: %i", this->temp_fitness), camera.getMainCamera().target.x - 470, camera.getMainCamera().target.y - 210, 20, WHITE);

	EndMode2D();
}

SceneType MainScene::setNextScene(ActorPlayer& player, std::vector<ActorAI>& ai_vector, bool& exitWindowRequested) {

	if (IsKeyPressed(KEY_BACKSPACE)) {
		return START_SCENE;
	}
	else if (IsKeyPressed(KEY_ESCAPE)) {
		exitWindowRequested = true;
	}

	return MAIN_SCENE;
}

//---------------------------Functions---------------------------------

void MainScene::drawBackground(MainCamera& camera) {
	Vector2 camera_position = camera.getMainCamera().target;
	camera_position.x = camera_position.x - static_cast<float>(GetScreenWidth() / 2);
	camera_position.y = camera_position.y - static_cast<float>(GetScreenHeight() / 2);
	for (const auto& tile : this->background_vector) {
		if (camera_position.x < tile->position_on_screen.x && tile->position_on_screen.x < camera_position.x + static_cast<float>(GetScreenWidth()) && camera_position.y < tile->position_on_screen.y && tile->position_on_screen.y < camera_position.y + static_cast<float>(GetScreenHeight())) {
			DrawTextureRec(this->tile_atlas_texture, tile->spritesheet_position, tile->position_on_screen, WHITE);
		}
	}

	if (IsKeyDown(KEY_ONE)) {
		for (const auto& tile : this->collider_vector) {
			if (camera_position.x < tile->collider_position.x && tile->collider_position.x < camera_position.x + static_cast<float>(GetScreenWidth()) && camera_position.y < tile->collider_position.y && tile->collider_position.y < camera_position.y + static_cast<float>(GetScreenHeight())) {
				switch (tile->id)
				{
				case 0:
					DrawRectangle(tile->collider_position.x, tile->collider_position.y, 16, 16, RED);
					break;
				case 1:
					DrawRectangle(tile->collider_position.x, tile->collider_position.y, 16, 16, GREEN);
					break;
				case 2:
					DrawRectangle(tile->collider_position.x, tile->collider_position.y, 16, 16, BLUE);
					break;
				default:
					break;
				}
			}
		}
	}
}

/*
void MainScene::drawForeground(MainCamera& camera) {
	Vector2 camera_position = camera.getMainCamera().target;
	camera_position.x = camera_position.x - static_cast<float>(GetScreenWidth() / 2);
	camera_position.y = camera_position.y - static_cast<float>(GetScreenHeight() / 2);
	for (const auto& tile : this->foreground_vector) {
		if (camera_position.x < tile->position_on_screen.x && tile->position_on_screen.x < camera_position.x + static_cast<float>(GetScreenWidth()) && camera_position.y < tile->position_on_screen.y && tile->position_on_screen.y < camera_position.y + static_cast<float>(GetScreenHeight())) {
			DrawTextureRec(this->tile_atlas_texture, tile->spritesheet_position, tile->position_on_screen, WHITE);
		}
	}
}
*/

void MainScene::parseLevelBackgroundTiles(nlohmann::json& tileset_description, nlohmann::json& level_map) {

	Vector2 vec = { 0, 0 };
	Rectangle rec = { 0, 0, level_map["tilewidth"], level_map["tileheight"] };

	for (auto const& layer : level_map["layers"]) {
		if (layer["type"] == "tilelayer" && layer["visible"]) {
			for (auto const& tileId : layer["data"]) {
				if (layer["id"] < 3) {
					if (tileId != 0) {
						rec.x = static_cast<float>(((static_cast<int>(tileId) -1) % static_cast<int>(tileset_description["columns"]))) * static_cast<float>(level_map["tilewidth"]);
						rec.y = static_cast<float>(floor(static_cast<float>(tileId) / static_cast<float>(tileset_description["columns"]))) * static_cast<float>(level_map["tilewidth"]);
						
						if (static_cast<int>(tileId) % 20 == 0) {
							rec.y -= 16;
						}

						std::shared_ptr<LevelTile> tile = std::make_shared<LevelTile>();
						tile->position_on_screen = vec;
						tile->spritesheet_position = rec;
						this->background_vector.push_back(tile);
					}
				}
				else {
					return;
				}

				vec.x += static_cast<float>(level_map["tilewidth"]);
				if (vec.x >= static_cast<float>(layer["width"]) * static_cast<float>(level_map["tilewidth"])) {
					vec.x = 0;
					vec.y += static_cast<float>(level_map["tileheight"]);
				}
				if (vec.y >= static_cast<float>(layer["height"]) * static_cast<float>(level_map["tileheight"])) {
					vec.y = 0;
				}
			}
		}

	}
}

/*void MainScene::parseLevelForegroundTiles(nlohmann::json& tileset_description, nlohmann::json& level_map) {
	Vector2 vec = { 0, 0 };
	Rectangle rec = { 0, 0, level_map["tilewidth"], level_map["tileheight"] };

	for (auto const& layer : level_map["layers"]) {
		if (layer["type"] == "tilelayer" && layer["visible"]) {
			for (auto const& tileId : layer["data"]) {
				if (layer["id"] > 8 && layer["id"] != 13) {
					if (tileId != 0) {
						rec.x = static_cast<float>(((static_cast<int>(tileId) - 1) % static_cast<int>(tileset_description["columns"]))) * static_cast<float>(level_map["tilewidth"]);
						rec.y = static_cast<float>(floor(static_cast<float>(tileId) / static_cast<float>(tileset_description["columns"]))) * static_cast<float>(level_map["tilewidth"]);
						if (static_cast<int>(tileId) % 32 == 0) {
							rec.y -= 16;
						}

						std::shared_ptr<LevelTile> tile = std::make_shared<LevelTile>();
						tile->position_on_screen = vec;
						tile->spritesheet_position = rec;
						this->foreground_vector.push_back(tile);
					}
				}

				vec.x += static_cast<float>(level_map["tilewidth"]);
				if (vec.x >= static_cast<float>(layer["width"]) * static_cast<float>(level_map["tilewidth"])) {
					vec.x = 0;
					vec.y += static_cast<float>(level_map["tileheight"]);
				}
				if (vec.y >= static_cast<float>(layer["height"]) * static_cast<float>(level_map["tileheight"])) {
					vec.y = 0;
				}
			}
		}

	}
}*/

void MainScene::parseLevelCollider(nlohmann::json& tileset_description, nlohmann::json& level_map) {
	Vector2 vec = { 0, 0 };
	Rectangle rec = { 0, 0, level_map["tilewidth"], level_map["tileheight"] };

	for (auto const& layer : level_map["layers"]) {
		if (layer["type"] == "tilelayer" && layer["id"] == 3) {
			for (auto const& tileId : layer["data"]) {
				if (tileId != 0) {
					std::shared_ptr<ColliderTile> collider_tile = std::make_shared<ColliderTile>();
					collider_tile->id = 0;
					collider_tile->collider_position = vec;
					this->collider_vector.push_back(collider_tile);
				}

				vec.x += static_cast<float>(level_map["tilewidth"]);
				if (vec.x >= static_cast<float>(layer["width"]) * static_cast<float>(level_map["tilewidth"])) {
					vec.x = 0;
					vec.y += static_cast<float>(level_map["tileheight"]);
				}
				if (vec.y >= static_cast<float>(layer["height"]) * static_cast<float>(level_map["tileheight"])) {
					vec.y = 0;
				}
			}
		}

	}
	for (auto const& layer : level_map["layers"]) {
		if (layer["type"] == "tilelayer" && layer["id"] == 4) {
			for (auto const& tileId : layer["data"]) {
				if (tileId != 0) {
					std::shared_ptr<ColliderTile> collider_tile = std::make_shared<ColliderTile>();
					collider_tile->id = 1;
					collider_tile->collider_position = vec;
					this->collider_vector.push_back(collider_tile);
				}

				vec.x += static_cast<float>(level_map["tilewidth"]);
				if (vec.x >= static_cast<float>(layer["width"]) * static_cast<float>(level_map["tilewidth"])) {
					vec.x = 0;
					vec.y += static_cast<float>(level_map["tileheight"]);
				}
				if (vec.y >= static_cast<float>(layer["height"]) * static_cast<float>(level_map["tileheight"])) {
					vec.y = 0;
				}
			}
		}

	}

	for (auto const& layer : level_map["layers"]) {
		if (layer["type"] == "tilelayer" && layer["id"] == 5) {
			for (auto const& tileId : layer["data"]) {
				if (tileId != 0) {
					std::shared_ptr<ColliderTile> collider_tile = std::make_shared<ColliderTile>();
					collider_tile->id = 2;
					collider_tile->collider_position = vec;
					this->collider_vector.push_back(collider_tile);
				}

				vec.x += static_cast<float>(level_map["tilewidth"]);
				if (vec.x >= static_cast<float>(layer["width"]) * static_cast<float>(level_map["tilewidth"])) {
					vec.x = 0;
					vec.y += static_cast<float>(level_map["tileheight"]);
				}
				if (vec.y >= static_cast<float>(layer["height"]) * static_cast<float>(level_map["tileheight"])) {
					vec.y = 0;
				}
			}
		}

	}
}

void MainScene::detectPlayerCollision(ActorPlayer& player, MainCamera& camera) {

	bool is_collision = false;
	Vector2 camera_position = camera.getMainCamera().target;
	camera_position.x = camera_position.x - static_cast<float>(GetScreenWidth() / 2);
	camera_position.y = camera_position.y - static_cast<float>(GetScreenHeight() / 2);

	Rectangle player_rec = { player.getCurrentPosition().x, player.getCurrentPosition().y, 16, 32 };
	Rectangle collider_rec = { 0, 0, 16, 16 };
	for (const auto& tile : this->collider_vector) {

		collider_rec.x = tile->collider_position.x;
		collider_rec.y = tile->collider_position.y;

		if (camera_position.x < collider_rec.x && collider_rec.x < camera_position.x + static_cast<float>(GetScreenWidth()) && camera_position.y < collider_rec.y && collider_rec.y < camera_position.y + static_cast<float>(GetScreenHeight())) {
			if (tile->id == 0 || tile->id == 1) {
				if (CheckCollisionRecs(player_rec, collider_rec)) {
					if (tile->collider_position.y < player.getCurrentPosition().y) { //Test if the platform is above the player
						player.setJumpVelocity(0);
						player.setCurrentPosition(player.getLastPosition());
					} else if (tile->collider_position.y > player.getCurrentPosition().y + 32 || tile->collider_position.y + 16 > player.getCurrentPosition().y + 32) { //Test if the platform is under the player
						player.setJumpVelocity(0);
						player.setCurrentPosition(player.getCurrentPosition().x, tile->collider_position.y - 32);
						player.setIsStanding(true);
					} else if (tile->collider_position.x < player.getCurrentPosition().x || tile->collider_position.x > player.getCurrentPosition().x + 16) { //Test if the platform is at the sides the player
						player.setIsStanding(false);
						player.setCurrentPosition(player.getLastPosition());
					}
					is_collision = true;
				} else if (is_collision == false) {
					player.setIsStanding(false);
				}
			} else if (tile->id == 2) {
				if (CheckCollisionRecs(player_rec, collider_rec)) {
					player.setIsDead(true);
				}
			}
		}
	}
	player.setLastPosition(player.getCurrentPosition());
}

void MainScene::detectAICollision(ActorAI& ai) {

	bool is_collision = false;

	Rectangle collider_rec = { 0, 0, 16, 16 };
	for (const auto& tile : this->collider_vector) {

			Rectangle ai_rec = { ai.getCurrentPosition().x, ai.getCurrentPosition().y, 16, 32};
			collider_rec.x = tile->collider_position.x;
			collider_rec.y = tile->collider_position.y;

			if (ai.getCurrentPosition().x - 128 < collider_rec.x && collider_rec.x < ai.getCurrentPosition().x + 128 && ai.getCurrentPosition().y - 128 < collider_rec.y && collider_rec.y < ai.getCurrentPosition().y + 128) {
				if (tile->id == 0 || tile->id == 1) {
					if (CheckCollisionRecs(ai_rec, collider_rec)) {
						if (tile->collider_position.y < ai.getCurrentPosition().y) { //Test if the platform is above the ai
							ai.setJumpVelocity(0);
							ai.setCurrentPosition(ai.getLastPosition());
						}
						else if (tile->collider_position.y > ai.getCurrentPosition().y + 32 || tile->collider_position.y + 16 > ai.getCurrentPosition().y + 32) { //Test if the platform is under the ai
							ai.setJumpVelocity(0);
							ai.setCurrentPosition(ai.getCurrentPosition().x, tile->collider_position.y - 32);
							ai.setIsStanding(true);
						}
						else if (tile->collider_position.x < ai.getCurrentPosition().x || tile->collider_position.x > ai.getCurrentPosition().x + 16) { //Test if the platform is at the sides the ai
							ai.setIsStanding(false);
							ai.setCurrentPosition(ai.getLastPosition());
						}
						is_collision = true;
					}
					else if (is_collision == false) {
						ai.setIsStanding(false);
					}
				}
				else if (tile->id == 2) {
					if (CheckCollisionRecs(ai_rec, collider_rec)) {
						ai.setIsDead(true);
						ai.setFitness(ai.getFitness() - DEATHPENALTY);
					}
				}
			}
		}
	ai.setLastPosition(ai.getCurrentPosition());
}

void MainScene::geneticAlgorithm(std::vector<ActorAI>& ai_vector) {
	ActorAI parent_ai1(true, NUMINPUTS, NUMHIDDEN, NUMOUTPUTS, ai_vector[0].getTexture());
	ActorAI parent_ai2(true, NUMINPUTS, NUMHIDDEN, NUMOUTPUTS, ai_vector[0].getTexture());
	ActorAI child_ai(true, NUMINPUTS, NUMHIDDEN, NUMOUTPUTS, ai_vector[0].getTexture());
	bool was_ai1_taken = false;

	srand(static_cast <unsigned> (time(0)));

	//Searches for the AI with the best fitness score
	for (int i = 0; i < ai_vector.size() - 1; i++) {
		if (parent_ai1.getFitness() < ai_vector[i].getFitness()) {
			parent_ai1 = ai_vector[i];
		} else if (parent_ai2.getFitness() < ai_vector[i].getFitness()) {
			parent_ai2 = ai_vector[i];
		}
	}

	for (int i = 0; i <= ai_vector.size() - 1; i++) {

		//Assignes the weight from the hidden layer of the two best AI in alternating order to the new AI
		for (int j = 0; j < NUMHIDDEN - 1; j++) {
			for (int k = 0; k < NUMINPUTS - 1; k++) {
				if (!was_ai1_taken) {
					if (GetRandomValue(0, 1000) < 1000 * MUTATIONRATE) {
						child_ai.getSynapseVectorHL1()[j][k].weight = MINWEIGHT + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (MAXWEIGHT - MINWEIGHT)));
					}
					else {
						child_ai.getSynapseVectorHL1()[j][k].weight = parent_ai1.getSynapseVectorHL1()[j][k].weight;
						was_ai1_taken = true;
					}
				}
				else {
					if (GetRandomValue(0, 1000) < 1000 * MUTATIONRATE) {
						child_ai.getSynapseVectorHL1()[j][k].weight = MINWEIGHT + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (MAXWEIGHT - MINWEIGHT)));
					}
					else {
						child_ai.getSynapseVectorHL1()[j][k].weight = parent_ai2.getSynapseVectorHL1()[j][k].weight;
						was_ai1_taken = false;
					}
				}
			}
		}

		//Assignes the weight from the output layer of the two best AI in alternating order to the new AI
		for (int j = 0; j < NUMOUTPUTS - 1; j++) {
			for (int k = 0; k < NUMHIDDEN - 1; k++) {
				if (!was_ai1_taken) {
					if (GetRandomValue(0, 1000) < 1000 * MUTATIONRATE) {
						child_ai.getSynapseVectorOL()[j][k].weight = MINWEIGHT + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (MAXWEIGHT - MINWEIGHT)));
					}
					else {
						child_ai.getSynapseVectorOL()[j][k].weight = parent_ai1.getSynapseVectorOL()[j][k].weight;
						was_ai1_taken = true;
					}
				}
				else {
					if (GetRandomValue(0, 1000) < 1000 * MUTATIONRATE) {
						child_ai.getSynapseVectorOL()[j][k].weight = MINWEIGHT + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (MAXWEIGHT - MINWEIGHT)));
					}
					else {
						child_ai.getSynapseVectorOL()[j][k].weight = parent_ai2.getSynapseVectorOL()[j][k].weight;
						was_ai1_taken = false;
					}
				}
			}
		}

		ai_vector[i] = child_ai;

	}
}

Output MainScene::findRightEdge(int iterator, float tile_position_x) {
	Output output;
	if (collider_vector[iterator].get()->id == 1 && tile_position_x + 16 == collider_vector[iterator].get()->collider_position.x) {
		output = findRightEdge(iterator + 1, collider_vector[iterator].get()->collider_position.x);
	} else {
		output.iterator = iterator;
		output.output = tile_position_x + 15;
		return output;
	}
	return output;
}