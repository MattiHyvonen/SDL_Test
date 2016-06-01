
#include "libnnInterface.h"
#include <iostream>
#include "dClock.h"

namespace nnInterface {

	NNet nn_net;

	std::vector<tilanne> tilanteet;

	std::vector<float> nn_output;
	std::vector<float> result;

	std::vector<float> nn_desired_out;
	std::vector<float> nn_input;

	std::atomic<bool> outRead;
	std::atomic<bool> inWritten;
	std::atomic<bool> desiredWritten;
	std::atomic<bool> nn_stop;
	std::atomic<bool> update;
	std::atomic<bool> kouluta;

	std::mutex mtx;

	dClock timer;

	int in = 2;
	int hid = 0;
	int out = 3;

	void Init()
	{

		// count num_inputs && num_hidden_neurons dynamically
		nn_desired_out = std::vector<float>(out, 0.5f);
		nn_input = std::vector<float>(in, 0.5f);
		nn_output = std::vector<float>(out, 0.5f);
		result = std::vector<float>(out, 0.5f);

		for (int i = in; i > 0; i--)
			hid += i;

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
			if (!kouluta) {
				mtx.lock();
				if (desiredWritten == true) {
					tilanteet.push_back(tilanne(nn_input, nn_desired_out));
					
					//voisi kirjoittaa tiedostoon tässä välissä
					desiredWritten = false;
				}
				mtx.unlock();

				std::this_thread::yield();

				mtx.lock();
				// feed forward
				if (inWritten == true) {
					nn_output = nn_net.forward(nn_input);
					if (nn_output.size() > out) {
						nn_output.resize(out);
						std::cerr << "liian iso output\n";						
					}
					outRead = false;
				}
				mtx.unlock();
				std::this_thread::yield();
			}

			else{ //koulutetaan annetuilla tilanteilla
				if (tilanteet.size() < 2)
					kouluta = false;
				else {
					mtx.lock();
					for (int i = 0; i < tilanteet.size(); i++) {
						nn_net.forward(tilanteet[i].inputData);
						nn_net.back(tilanteet[i].desiredOutData);
					}
					nn_output = nn_net.forward(tilanteet.back().inputData);
					outRead = false;
					mtx.unlock();
					std::this_thread::yield();
				}
			}
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

		//std::cout << "teach\n";

		outRead = true;
		result = nn_output;
		return result;

	}


	void SetDesiredOut(std::vector<float> desired_out_)
	{
		if (desired_out_.size() > out)
			desired_out_.resize(out);

		nn_desired_out = desired_out_;
		desiredWritten = true;

	}


	void SetKouluta(bool value) {
		kouluta = value;
	}

    void LaskeDesiredOut (std::vector<float> nykyinenPaikka)
    {
        std::vector<float> erot(tilanteet.size());
        
        for(int i = 0; i < tilanteet.size(); i++)
            erot[i] = vektorienEro(nn_input, tilanteet[i].inputData);
        
        float pieninEro = 10000000;
        float lahinTilanneId;
        
        for(int i = 0; i < erot.size(); i++)
            if(erot[i] < pieninEro) {
                pieninEro = erot[i];
                lahinTilanneId = i;
            }
        
        nn_desired_out = tilanteet[lahinTilanneId].desiredOutData;
        
        for(int i = 0; i < nykyinenPaikka.size(); i++)
            nn_desired_out[i] = nykyinenPaikka[i] - nn_desired_out[i];
        
    
    }

	void Close() {
		nn_stop = true;
	}
}