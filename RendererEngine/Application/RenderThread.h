#pragma once

#include <thread>

namespace app {

struct RenderThread {

	void start();
	void stop();

	void run();

	std::thread thread;
};

}
