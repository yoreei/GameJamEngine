#pragma once
#include <cstdint>
#include <vector>
#include <memory>
#include <bitset>
#include <random>

#include "Animation.h"
#include "3d/common3d.h"





enum class EntityType {
	PlayerEntity1 = 0,
	PlayerEntity2,
	PlayerEntity3,
	PlayerEntity4,
	EnemyEntity1,
	EnemyEntity2,
	EnemyEntity3,
	EnemyEntity4
};

enum class Belonging {
	Player = 0,
	CPU
};
struct Entity {
	uint32_t health = 1;
	//Belonging belonging = Belonging::CPU;
	//uint8_t factionId = 0;
	//uint32_t maxHealth = 1;
	//uint32_t power = 1;
	//uint32_t maxPower = 1;
	//std::bitset<32> powerUps;
	//std::vector<size_t> bitmapIds{};
	//Animation animation = Animation::Idle;
	vec3 position;
	//vec3 direction;
	vec3 momentum;
};

class Obstacle {

};

enum class State {
	MAINMENU = 0,
	INGAME,
	PAUSED,
	LOSS,
	WIN,
	size
};
struct GJScene {
	GJScene() {
	}

	void resetEntities() {
		entities.fill(GJScene::emptyEntity);
		for (int i = 0; i < entities.size(); ++i) {
			entities[i].health = 1;
			entities[i].position = vec3{ 180.f, 240.f, 0.f };
		}
		//keybinds['Q'] = 0;
		//keybinds['A'] = 0;
		//keybinds['W'] = 0;
		//keybinds['S'] = 0;
	}

	void resetObstacles() {
		obstacles.fill(GJScene::emptyEntity);
	}

	void initRandObstacle(size_t id) {
		static std::mt19937 GEN(46);
		static std::uniform_real_distribution<float> posDistr(-20.f, 380.f);
		static std::uniform_int_distribution<int> sideDistr(0, 3);

		obstacles[id].health = 1;
		int side = sideDistr(GEN);
		if (side == 0) { // top
			obstacles[id].position.e[0] = posDistr(GEN); //< set y
			obstacles[id].position.e[1] = 0.f;
		}
		else if (side == 1) { // right
			obstacles[id].position.e[1] = posDistr(GEN); //< set y
			obstacles[id].position.e[0] = 380.f;

		}
		else if (side == 2) { // bottom
			obstacles[id].position.e[0] = posDistr(GEN); //< set y
			obstacles[id].position.e[1] = 380.f;

		}
		else { // left
			obstacles[id].position.e[1] = posDistr(GEN); //< set y
			obstacles[id].position.e[0] = 0.f;

		}
		vec3 center{ 180.f, 180.f, 0.f };
		obstacles[id].momentum = center - obstacles[id].position;
		obstacles[id].momentum = unit_vector(obstacles[id].momentum);

	}

	//std::array<int, 255> keybinds;
	State state = State::MAINMENU;
	Entity playerController;
	//v entities move
	std::array<Entity, 4> entities{};
	std::array<Entity, 20> obstacles{};
	static inline Entity emptyEntity;
};
