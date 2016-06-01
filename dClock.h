#ifndef DCLOCK_H
#define DCLOCK_H

#include <time.h>
#include <thread>
#include <chrono>

class dClock{
public:
	clock_t lastTime;

	void reset() { lastTime = clock(); }

	float get() {
		clock_t time = clock() - lastTime;
		return ((float)time / CLOCKS_PER_SEC);
	}

	void delay_ms(int ms) {
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	}

	bool delay(float target_fps) {
		int ms = (1000 / target_fps) - get() * 1000;
		reset();

//		using namespace std::this_thread; // sleep_for, sleep_until
//		using namespace std::chrono; // nanoseconds, system_clock, seconds

		if (ms > 0) {
			//if (ms > 1000)
			//	ms = 1000;
			
			delay_ms(ms);
			return true;
		}

		else return false; //if lag, return false
	}



	dClock(){
		reset();
	}

};

#endif