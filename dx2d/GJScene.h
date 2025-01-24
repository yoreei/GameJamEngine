#pragma once
#include <cstdint>
#include <vector>
#include <memory>
#include <bitset>

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
class Entity {
	Belonging belonging = Belonging::CPU;
	uint8_t factionId = 0;
	uint32_t health = 1;
	uint32_t maxHealth = 1;
	uint32_t power = 1;
	uint32_t maxPower = 1;
	std::bitset<32> powerUps;
	std::vector<size_t> bitmapIds{};
	Animation animation = Animation::Idle;
	vec3 position;
	vec3 direction;
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
	State state = State::MAINMENU;
	Entity playerController;
	//v entities move
	std::vector<std::unique_ptr<Entity>> entities{};
	//v obstacles do not move
	std::vector<std::unique_ptr<Obstacle>> obstacles{};
};
