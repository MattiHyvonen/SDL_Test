#ifndef __libnn__libnnInterface__
#define __libnn__libnnInterface__

#include <stdio.h>
#include "libnn.h"
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include <string>


namespace nnInterface {
extern std::mutex mtx;

struct tilanne{
	std::vector<float> inputData;
	std::vector<float> desiredOutData;

	tilanne(){}
	tilanne(std::vector<float> input, std::vector<float> desired) : inputData(input), desiredOutData(desired) {}
	std::string toString() {
		std::string vastaus = "inputs: ";
		for (int i = 0; i < inputData.size(); i++) {
			vastaus += " " + std::to_string(inputData[i]);
		}
		vastaus += "\n";
		for (int i = 0; i < desiredOutData.size(); i++) {
			vastaus += " " + std::to_string(desiredOutData[i]);
		}
		vastaus += "\n";
		return vastaus;
	}
};

void Init();

void StartRoutine();

void SetInput(std::vector<float> input_);

std::vector<float> GetOutput();

void SetDesiredOut(std::vector<float> desired_out_);

void SetKouluta(bool);

void Close();
}
#endif
