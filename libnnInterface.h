#ifndef __libnn__libnnInterface__
#define __libnn__libnnInterface__

#include <stdio.h>
#include "libnn.h"
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>


namespace nnInterface {
extern std::mutex mtx;


void Init();

void StartRoutine();

void SetInput(std::vector<float> input_);

std::vector<float> GetOutput();

void SetDesiredOut(std::vector<float> desired_out_);

void Close();
}
#endif
