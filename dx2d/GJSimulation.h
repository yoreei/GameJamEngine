#pragma once
#include <chrono>
#include <bitset>
#include <stdexcept>
#include "GJScene.h"

class GJSimulation {
public:
	GJSimulation(GJScene* scene) : scene(scene) {}
	void tick(std::chrono::duration<double, std::milli> delta) {
		if (scene->state != State::INGAME) {
			return;
		}

		if (kbMap[VK_LEFT]) {
			// handle left key here
		}

		
	}

	void handleInput(WPARAM wParam, bool keyDown) {
		if (wParam > 254) {
			throw std::runtime_error("unexpected key value");
		}
		kbMap[wParam] = keyDown;
		if (wParam == VK_ESCAPE) {
			scene->state = State::MAINMENU;
		}

	}

	void restart() {

	}

	void updateState() {

	}

	void triggerLoss() {

	}

private:
	std::bitset<254> kbMap;
	GJScene* scene;
};
