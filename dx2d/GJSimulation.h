#pragma once
#include <chrono>
#include "GJScene.h"

class GJSimulation {
public:
	GJSimulation(GJScene* scene) : scene(scene) {}
	void tick(std::chrono::duration<double, std::milli> delta) {
		if (scene->state == State::PLAYING) {

		}

		
	}

	void handleInput() {
		// if key ESC:
		scene->state = State::MAINMENU;
	}

	void restart() {

	}

	void updateState() {

	}

	void triggerLoss() {

	}

private:
	GJScene* scene;
};
