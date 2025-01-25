#pragma once
#include <chrono>
#include <bitset>
#include <stdexcept>

#include "GJScene.h"
#include "irrKlang.h"

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

		lastExplode = std::chrono::high_resolution_clock::now();
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
			auto now = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> cooldownDuration{ 1000.f * cooldownSeconds };
			if (now - lastExplode < cooldownDuration) { return; }
			else {
				lastExplode = now;

				if (wParam == 'Q') {
					target = 0;
				}
				else if (wParam == 'W') {
					target = 1;
				}
				else if (wParam == 'A') {
					target = 2;
				}
				else if (wParam == 'S') {
					target = 3;
				}
				else {
					return;
				}
				for (int i = 0; i < scene.entities.size(); ++i) {
					scene.entities[i].health = 1;
					scene.entities[i].position = scene.entities[target].position;
				}
				static vec3 NW = unit_vector(vec3{ -1, -1, 0 });
				static vec3 NE = unit_vector(vec3{ 1, -1, 0 });
				static vec3 SE = unit_vector(vec3{ 1, 1, 0 });
				static vec3 SW = unit_vector(vec3{ -1, 1, 0 });
				scene.entities[0].momentum = NW;
				scene.entities[1].momentum = NE;
				scene.entities[2].momentum = SE;
				scene.entities[3].momentum = SW;

			}

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
		for (int i = 0; i < scene.entities.size(); ++i) {
			Entity& e = scene.entities[i];
			if (e.health <= 0) {
				continue;
			}
			e.position += e.momentum;

			// wraparound mechanic
			if (e.position.x() < 0.f) { e.position.e[0] = 359.f; }
			if (e.position.x() > 360.f) { e.position.e[0] = 1.f; }
			if (e.position.y() < 0.f) { e.position.e[1] = 359.f; }
			if (e.position.y() > 360.f) { e.position.e[1] = 1.f; }

		}
		for (int i = 0; i < scene.obstacles.size(); ++i) {
			Entity& e = scene.obstacles[i];
			if (e.health <= 0) {
				continue;
			}
			e.position += e.momentum;

			// wraparound mechanic
			if (e.position.x() < 0.f) { e.position.e[0] = 359.f; }
			if (e.position.x() > 360.f) { e.position.e[0] = 1.f; }
			if (e.position.y() < 0.f) { e.position.e[1] = 359.f; }
			if (e.position.y() > 360.f) { e.position.e[1] = 1.f; }

		}
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
			scene.initRandObstacle(i);
		}
		enterINGAME();
	}

private:
	double cooldownSeconds = 1.0;
	std::chrono::time_point<std::chrono::high_resolution_clock> lastExplode;
	GJScene scene;
	std::bitset<254> kbMap;
	irrklang::ISoundEngine* audioEngine;
	using KeybindHandler = void(GJSimulation::*)(WPARAM, bool);
	std::array<KeybindHandler, static_cast<size_t>(State::size)> kbCallTable;

};
