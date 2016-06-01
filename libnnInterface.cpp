
#include "libnnInterface.h"
#include <iostream>
#include "dClock.h"
#include "random.h"

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
				mtx.lock();
				if (desiredWritten == true) {
                    nn_net.back(nn_desired_out);
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
    
    void TeeTilanne (std::vector<float> input, std::vector<float> output)
    {
        std::cout << "input: " << input[0] << " " << input[1] << "\n";
        std::cout << "output: " << output[0] << " " << output[1] << " "<< output[2]<< "\n";
        
        tilanteet.push_back(tilanne(input,output));
    
    
    }

    void LaskeDesiredOut (std::vector<float> nykyinenPaikka)
    {
        std::vector<float> erot(tilanteet.size());
        
        std::this_thread::yield();
        
        
        if(nn_input.size() == in) {
            for(int i = 0; i < tilanteet.size(); i++) {
                erot[i] = vektorienEro(nn_input, tilanteet[i].inputData);
            }
        
        float pieninEro = 10000000;
        int lahinTilanneId = 0;
        
        for(int i = 0; i < erot.size(); i++)
            if(erot[i] < pieninEro) {
                pieninEro = erot[i];
                lahinTilanneId = i;
            }
            std::cout << "\n";
        
        std::cout << "pieninEro: " << pieninEro << "\n";
        std::cout << "desired: " ;
            

            
        for(int i = 0; i < nykyinenPaikka.size(); i++) {
            nn_desired_out[i] =  (tilanteet[lahinTilanneId].desiredOutData[i] - nykyinenPaikka[i]) * 10;
            bound(nn_desired_out[i],-1,1);
            //nn_desired_out[i] = tilanteet[lahinTilanneId].desiredOutData[i];
            std::cout << nn_desired_out[i] << " ";
        }
        
        std::cout << "\n";
        
        desiredWritten = true;
        }
        else
            std::cout << "vaaran kokoinen nn_input: " << nn_input.size();
        
        std::cout << "\n";
    }

	void Close() {
		nn_stop = true;
	}
}