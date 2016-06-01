
#include "libnnInterface.h"
#include <iostream>
#include "dClock.h"

namespace nnInterface {

	NNet nn_net;

	std::vector<float> nn_output;
	std::vector<float> result;

	std::vector<float> nn_desired_out;
	std::vector<float> nn_input;

	std::atomic<bool> outRead;
	std::atomic<bool> inWritten;
	std::atomic<bool> desiredWritten;
	std::atomic<bool> nn_stop;
	std::atomic<bool> update;

	std::mutex mtx;

	dClock timer;

	void Init()
	{

		// count num_inputs && num_hidden_neurons dynamically
		int in = 2;
		int hid = 3;
		int out = 3;

		nn_desired_out = std::vector<float>(out, 0.5f);
		nn_input = std::vector<float>(in, 0.5f);
		nn_output = std::vector<float>(out, 0.5f);
		result = std::vector<float>(out, 0.5f);

		nn_net.init(in, 1, hid, out);

		//tee linkitykset
		int reduction = 0;
		for (int i = 0; i < in; i++) {
			reduction += i;
			for (int j = 0; j < in; j++) {
				nn_net.link(0, i, 1, i*in + j - reduction);
				nn_net.link(0, j, 1, i*in + j - reduction);
			}
		}

	}

	void StartRoutine() {

		// aloita looppi
		nn_stop = false;
		while (!nn_stop) {
			// back propagation

			mtx.lock();
			if (desiredWritten == true) {
				nn_net.back(nn_desired_out);
				desiredWritten = false;
			}
			mtx.unlock();

			std::this_thread::yield();

			mtx.lock();
			// feed forward
			if (inWritten == true) {
				nn_output = nn_net.forward(nn_input);
				outRead = false;
			}
			mtx.unlock();
			//    }
			std::this_thread::yield();
		}
	}

	void SetInput(std::vector<float> input_)
	{
		nn_input = input_;
		inWritten = true;
	}

	std::vector<float> GetOutput()
	{
		if (outRead) {
			result.clear();
			return result;
		}

		std::cout << "teach\n";

		outRead = true;
		result = nn_output;
		return result;

	}

	void SetDesiredOut(std::vector<float> desired_out_)
	{

		nn_desired_out = desired_out_;
		desiredWritten = true;

	}


	void Close() {
		nn_stop = true;
	}
}