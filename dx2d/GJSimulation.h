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

		enterMAINMENU();
	}

	const GJScene* getScene() {
		return &scene;
	}

	void kbHandleINGAME(WPARAM wParam, bool keyDown) {
		if (keyDown) {
			if (wParam == VK_ESCAPE) {
				enterPAUSED();
			}
			else if (wParam == 'Q') {
				// handle left key here
			}
			else if (wParam == 'W') {
				// handle left key here
			}
			else if (wParam == 'A') {
				// handle left key here
			}
			else if (wParam == 'S') {
				// handle left key here
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
			if (e.position.y() < 0.f) { e.position.e[0] = 359.f; }
			if (e.position.y() > 360.f) { e.position.e[0] = 1.f; }

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
			if (e.position.y() < 0.f) { e.position.e[0] = 359.f; }
			if (e.position.y() > 360.f) { e.position.e[0] = 1.f; }

		}
	}

	void handleInput(WPARAM wParam, bool keyDown) {
		//if (wParam > 254) {
		//	throw std::runtime_error("unexpected key value");
		//}
		//kbMap[wParam] = keyDown; 
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
	GJScene scene;
	std::bitset<254> kbMap;
	irrklang::ISoundEngine* audioEngine;
	using KeybindHandler = void(GJSimulation::*)(WPARAM, bool);
	std::array<KeybindHandler, static_cast<size_t>(State::size)> kbCallTable;

};
