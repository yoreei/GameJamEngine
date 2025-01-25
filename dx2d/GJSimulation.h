#pragma once
#include <chrono>
#include <bitset>
#include <stdexcept>

#include "GJScene.h"
#include "irrKlang.h"
#include "GJGlobals.h"

class GJSimulation {
public:
	GJSimulation() {
		audioEngine = irrklang::createIrrKlangDevice();
		if (!audioEngine) {
			MessageBox(NULL, L"Could not initialize audio engine.", L"Error", MB_OK);
		}
		kbCallTable[static_cast<size_t>(State::INGAME)] = &GJSimulation::kbHandleINGAME;
		kbCallTable[static_cast<size_t>(State::WIN)] = &GJSimulation::kbHandleWIN;
		kbCallTable[static_cast<size_t>(State::LOSS)] = &GJSimulation::kbHandleLOSS;
		kbCallTable[static_cast<size_t>(State::PAUSED)] = &GJSimulation::kbHandlePAUSED;
		kbCallTable[static_cast<size_t>(State::MAINMENU)] = &GJSimulation::kbHandleMAINMENU;

		lastExplode = getTime();
		enterMAINMENU();
	}

	const GJScene* getScene() {
		return &scene;
	}

	void kbHandleINGAME(WPARAM wParam, bool keyDown) {
		if (keyDown) {
			if (wParam == VK_ESCAPE) {
				enterPAUSED();
				return;
			}

			size_t target = 0;

			if (wParam == 'Q' && scene.entities[0].health > 0) {
				target = 0;
			}
			else if (wParam == 'W' && scene.entities[1].health > 0) {
				target = 1;
			}
			else if (wParam == 'S' && scene.entities[2].health > 0) {
				target = 2;
			}
			else if (wParam == 'A' && scene.entities[3].health > 0) {
				target = 3;
			}
			else {
				return;
			}

			if (scene.cooldown) { return; }

			lastExplode = GNow;
			scene.cooldown = true;
			for (int i = 0; i < scene.entities.size(); ++i) {
				scene.entities[i].health = 1;
				scene.entities[i].setPos(scene.entities[target].getPos());
			}
			static vec3 NW = unit_vector(vec3{ -1, -1, 0 });
			static vec3 NE = unit_vector(vec3{ 1, -1, 0 });
			static vec3 SE = unit_vector(vec3{ 1, 1, 0 });
			static vec3 SW = unit_vector(vec3{ -1, 1, 0 });
			scene.entities[0].momentum = NW; // Q
			scene.entities[1].momentum = NE; // W
			scene.entities[2].momentum = SE; // S
			scene.entities[3].momentum = SW; // A


		}
	}

	void kbHandleWIN(WPARAM wParam, bool keyDown) {
		kbHandlePAUSED(wParam, keyDown);
	}

	void kbHandleLOSS(WPARAM wParam, bool keyDown) {
		kbHandlePAUSED(wParam, keyDown);
	}

	void kbHandlePAUSED(WPARAM wParam, bool keyDown) {
		if (keyDown) {
			if (wParam == VK_ESCAPE) {
				enterINGAME();
			}
			else if (wParam == 'R') {
				loadNewGame();
			}
			else if (wParam == VK_BACK) {
				exit(0);
			}
		}
	}

	void kbHandleMAINMENU(WPARAM wParam, bool keyDown) {
		if (keyDown) {
			if (wParam == VK_RETURN) {
				loadNewGame();
			}
			else if (wParam == VK_BACK) {
				exit(0);
			}
		}

	}

	void enterMAINMENU() {
		scene.state = State::MAINMENU;

		audioEngine->stopAllSounds();
		auto music = audioEngine->play2D("assets/mainmenu.mp3", true, false, true);
		if (!music) {
			MessageBox(NULL, L"Could not play mainmenu.mp3", L"Error", MB_OK);
		}
	}

	void enterWIN() {
		scene.state = State::WIN;
		audioEngine->stopAllSounds();
		auto music = audioEngine->play2D("assets/win.mp3", true, false, true);
		if (!music) {
			MessageBox(NULL, L"Could not play win.mp3", L"Error", MB_OK);
		}
	}

	void enterLOSS() {
		scene.state = State::LOSS;
		audioEngine->stopAllSounds();
		auto music = audioEngine->play2D("assets/loss.mp3", true, false, true);
		if (!music) {
			MessageBox(NULL, L"Could not play loss.mp3", L"Error", MB_OK);
		}
	}

	void enterPAUSED() {
		scene.state = State::PAUSED;

		//audioEngine->stopAllSounds();
		//auto music = audioEngine->play2D("assets/mainmenu.mp3", true, false, true);
		//if (!music) {
		//	MessageBox(NULL, L"Could not play mainmenu.mp3", L"Error", MB_OK);
		//}

	}

	void enterINGAME() {
		scene.state = State::INGAME;

	}

	void tick(std::chrono::duration<double, std::milli> delta) {
		if (scene.state != State::INGAME) {
			return;
		}
		tickStats(delta);
		tickMovement(delta);
		tickCollision(delta);
		++tickCounter;
	}

	void tickStats(std::chrono::duration<double, std::milli> delta) {
		if (scene.cooldown && GNow - lastExplode > cooldownDuration) {
			scene.cooldown = false;
		}
		auto lastPointsTimeDelta = std::chrono::duration_cast<std::chrono::seconds>(GNow - lastPointsTime);
		if (lastPointsTimeDelta.count() > 2) {
			lastPointsTime = GNow;
			scene.points += 100;

		}
	}

	void wrapAround(Entity& e) {
		vec3 pos = e.getPos();
		if (pos.x() < 0.f) { e.setX(359.f); }
		if (pos.x() > 360.f) { e.setX(1.f); }
		if (pos.y() < 0.f) { e.setY(359.f); }
		if (pos.y() > 360.f) { e.setY(1.f); }

	}

	void tickMovement(std::chrono::duration<double, std::milli> delta) {
		for (int i = 0; i < scene.entities.size(); ++i) {
			Entity& e = scene.entities[i];
			if (e.health <= 0) {
				continue;
			}
			e.moveBy(e.momentum * globalSpeedUp);

			// wraparound mechanic
			wrapAround(e);


		}
		for (int i = 0; i < scene.obstacles.size(); ++i) {
			Entity& e = scene.obstacles[i];
			if (e.health <= 0) {
				continue;
			}
			e.moveBy(e.momentum * globalSpeedUp);

			wrapAround(e);

		}
	}

	void tickCollision(std::chrono::duration<double, std::milli> delta) {
		for (int i = 0; i < scene.obstacles.size(); ++i) {
			Entity& o1 = scene.obstacles[i];
			if (o1.health <= 0) {
				continue;
			}
			for (int j = i + 1; j < scene.obstacles.size(); ++j) {
				Entity& o2 = scene.obstacles[j];
				if (o2.health <= 0) {
					continue;
				}
				if (isCollision(o1, o2)) {
					ricochet(o1, o2);
				}

			}
			for (int j = 0; j < scene.entities.size(); ++j) {
				Entity& e = scene.entities[j];
				if (e.health <= 0) {
					continue;
				}
				if (isCollision(o1, e, cheatFactor)) {
					killEntity(j);
				}
			}

		}
	}

	void ricochet(Entity& o1, Entity& o2) {
		//o1
		vec3 away = o1.getPos() - o2.getPos();
		// size is equivalent to weight
		o1.momentum = unit_vector(away + (o1.momentum * o1.size));

		away = -away;
		o2.momentum = unit_vector(away + (o2.momentum * o2.size));
	}


	void killEntity(size_t id) {
		scene.entities[id].health = 0;
		for (const Entity& e : scene.entities) {
			if (e.health != 0) {
				return;
			}
		}
		// here all entities have health 0
		scene.entities[id].health = 1; //< so the player can see their entity on the loss screen
		enterLOSS();
	}

	void handleInput(WPARAM wParam, bool keyDown) {

		(this->*kbCallTable[static_cast<size_t>(scene.state)])(wParam, keyDown);

	}

	void loadNewGame() {
		audioEngine->stopAllSounds();
		auto music = audioEngine->play2D("assets/ingame.mp3", true, false, true);
		if (!music) {
			MessageBox(NULL, L"Could not play ingame.mp3", L"Error", MB_OK);
		}

		scene.resetEntities();
		scene.resetObstacles();
		for (int i = 0; i < scene.obstacles.size(); ++i) {
			initRandObstacle(i);
		}
		GGameStart = getTime();
		enterINGAME();
	}

	void initRandObstacle(size_t id) {
		static std::mt19937 GEN(46);
		static std::uniform_int_distribution<int> posDistr(-20, 380);
		static std::uniform_int_distribution<int> sideDistr(0, 3);

		auto& obstacles = scene.obstacles;
		obstacles[id].health = 1;
		bool unique = false;
		while (!unique) {

			int side = sideDistr(GEN);
			double pos = static_cast<double>(posDistr(GEN));
			if (side == 0) { // top
				vec3 newPos{ pos, 0, 0 };
				obstacles[id].setPos(newPos);
			}
			else if (side == 1) { // right
				vec3 newPos{ 380, pos, 0 };
				obstacles[id].setPos(newPos);
			}
			else if (side == 2) { // bottom
				vec3 newPos{ pos, 380.f, 0 };
				obstacles[id].setPos(newPos);
			}
			else { // left
				vec3 newPos{ 0, pos, 0 };
				obstacles[id].setPos(newPos);
			}

			unique = true;
			for (int i = 0; i < obstacles.size(); ++i) {
				Entity& o1 = scene.obstacles[i];
				if (o1.health <= 0 || i == id) {
					continue;
				}
				if (isCollision(obstacles[id], obstacles[i])) {
					unique = false;
					break;
				}
			}
		}

		vec3 center{ 180.f, 180.f, 0.f };
		obstacles[id].momentum = center - obstacles[id].getPos();
		obstacles[id].momentum = unit_vector(obstacles[id].momentum);

		static std::uniform_int_distribution<uint16_t> sizeDist(9, 13);
		obstacles[id].size = sizeDist(GEN);

	}

	bool isCollision(const Entity& e1, const Entity& e2, float cheatParam = 0.f) {
		vec3 delta = e1.getPos() - e2.getPos();
		//vec3 delta = e1.position - e2.position;
		float collisionLim = e1.size + e2.size - cheatParam;
		if (std::abs(delta.e[0]) < collisionLim) {
			if (std::abs(delta.e[1]) < collisionLim) {
				return true;
			}
		}
		return false;
	}


private:
	float cheatFactor = 1.f;
	float globalSpeedUp = 2.f;
	uint64_t tickCounter = 0;
	double cooldownSeconds = 1.0;
	std::chrono::duration<double, std::milli> cooldownDuration{ 1000.f * cooldownSeconds };
	std::chrono::time_point<std::chrono::high_resolution_clock> lastExplode;
	std::chrono::time_point<std::chrono::high_resolution_clock> lastPointsTime;
	GJScene scene;
	std::bitset<254> kbMap;
	irrklang::ISoundEngine* audioEngine;
	using KeybindHandler = void(GJSimulation::*)(WPARAM, bool);
	std::array<KeybindHandler, static_cast<size_t>(State::size)> kbCallTable;

};
