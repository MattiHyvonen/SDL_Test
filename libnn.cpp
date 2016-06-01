//
//  libnn.cpp
//  libnn
//
//  Created by Ismo Torvinen on 23.5.2016.
//  Copyright (c) 2016 Ismo Torvinen. All rights reserved.
//

#include "libnn.h"
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>

//SIGMOID
const float SIGMOID_RESPONSE = 0.5;


void Sigmoid(float* netinput)
{
    *netinput = ( 1 / ( 1 + exp(-(*netinput) / SIGMOID_RESPONSE)));
}

//----------------------------------------
// argumentiksi annetaan Sigmoid(x) => palauttaa Sigmoid’ (x)

float deltaSigmoid(float sig_x)
{
    return (sig_x ) * (1 - sig_x );
}

//-------------------------

Neuron::Neuron () :
    error(0),
    output(0)
{
}


HiddenNeuron::HiddenNeuron (float learn_rate)
{
    learnRate = learn_rate;
    weights.push_back(1.0f);
}

void HiddenNeuron::forward()
{
    output = 0;
    
    output += 1 * weights[0];  // bias
    
    for(int i =0;i < upper.size();i++)
    {
        output += upper[i]->output * weights[i+1]; //ohitetaan bias lisäämällä weights indeksiin +1
    }
    
    Sigmoid(&output);

}


void HiddenNeuron::back()
{

    weights[0] = weights[0] + learnRate * error * 1; // bias
    
    for(int i = 0; i < upper.size(); i++)
    {
        upper[i]->error += deltaSigmoid (output) * error;
        weights[i+1] = weights[i+1] + learnRate * upper[i]->error * upper[i]->output;
    }
    
}

void HiddenNeuron::setInput(float value)
{
    std::cerr << "don't use this set input function!!!\n";
    upper[0]->output = value;
}

void HiddenNeuron::addLink(Neuron* neuron)
{
    upper.push_back(neuron);
    weights.push_back(0.0f);
}


InputNeuron::InputNeuron(float learn_rate) :
    input(0)
{
    learnRate = learn_rate;
    weights.push_back(1); // bias
    weights.push_back(0); // input
}

void InputNeuron::forward()
{
    output = 0;
    
    output += 1 * weights[0];  // bias
    output += input * weights[1]; // input
    
    Sigmoid(&output);
    
}

void InputNeuron::back()
{
    weights[0] = weights[0] + learnRate * error * 1; // bias
    weights[1] = weights[1] + learnRate * error * input;

}

void InputNeuron::setInput(float value)
{
    input = value;
}

void InputNeuron::addLink(Neuron* neuron)
{
    std::cerr << "cannot link input layer with addLink command\n";
}
// end of Neuron














// NLayer

NLayer::NLayer ()
{
}

void NLayer::init(int neurons_n, bool is_input_layer)
{
    if(!is_input_layer)
        for (int i = 0; i < neurons_n; i++) neurons.push_back(new HiddenNeuron());
    else
        for (int i = 0; i < neurons_n; i++) neurons.push_back(new InputNeuron());
}


void NLayer::forward()
{
    for(int i = 0; i < neurons.size(); i++)
    {
        neurons[i]->forward();
    }
}

void NLayer::back()
{
    for(int i = 0; i < neurons.size(); i++)
    {
        neurons[i]->back();
    }

}

void NLayer::setInputs(std::vector<float> inputs)
{
    
    
    if(inputs.size() == neurons.size())
        for(int i = 0; i < neurons.size();i++)
            neurons[i]->setInput(inputs[i]);
    else
        std::cerr << "wrong input vector size!!!\n";
}

std::vector<float> NLayer::getOutput()
{
    std::vector<float> result;
    
    for(int i = 0; i < neurons.size(); i++)
        result.push_back(neurons[i]->output);

    return result;
}

void NLayer::setError(std::vector<float> errors)
{
    
    if (errors.size() == neurons.size()) {
        for (int i = 0; i < neurons.size(); i++) {
            neurons[i]->error =
            deltaSigmoid(neurons[i]->output) *
            (errors[i] - neurons[i]->output);
        }
    }
    else
    {
        std::cerr << "wrong errors vector size!!\n";
    }

}

void NLayer::clearErrors()
{
    for(int i = 0; i < neurons.size();i++)
    {
        neurons[i]->error = 0;
    }
}

std::vector<NeuronData> NLayer::getNeuronData ()
{
    std::vector<NeuronData> data;
    for(int i = 0; i < neurons.size(); i++)
        data.push_back(*neurons[i]);

    return data;
}

// end of NLAYER

NNetData::NNetData(){}


std::string NNetData::toString ()
{
    std::stringstream ss;

    ss << inputs_n << " " << hidden_layers_n << " " << hidden_neurons_n << " " << outputs_n << std::endl;
    
    for(int i = 0; i < nData.size(); i++) {
        ss << "n" << std::endl;
        
        for(int j = 0; j < nData[i].weights.size(); j++) {
            ss << nData[i].weights[j];
            ss << " ";
        }
        
        ss << nData[i].output;
        ss << std::endl;
        
    }
    
    for(int i = 0; i < lData.size();i++)
    {
        ss << "l" << std::endl;
        ss << lData[i].source_layer << " " << lData[i].source_id << " " << lData[i].dest_layer << " " << lData[i].dest_id << std::endl;
    }
    
    return ss.str();
    
}






// NNET

NNet::NNet()
{
}

void NNet::init(int inputs_n, int hidden_layers_n, int hidden_neurons_n, int outputs_n)
{
    inputLayer.init(inputs_n, true);
    outputLayer.init(outputs_n);

    for(int i = 0; i < hidden_layers_n; i++) {
        hiddenLayers.push_back(NLayer());
        hiddenLayers.back().init(hidden_neurons_n);
    }
    
    // link all neurons from last hidden layer to every output layer's neuron.
    for(int i= 0; i < outputs_n; i++) {
        for(int j=0; j < hidden_neurons_n; j++) {
            outputLayer.neurons[i]->addLink(hiddenLayers.back().neurons[i]);
        }
    }
    
    nnData = NNetData(inputs_n, hidden_layers_n, hidden_neurons_n, outputs_n);
    
}


std::vector<float> NNet::forward(std::vector<float> &inputs)
{
    inputLayer.setInputs(inputs);
    inputLayer.forward();
    
    for(int i = 0; i < hiddenLayers.size(); i++)
        hiddenLayers[i].forward();
    
    outputLayer.forward();
    
    return outputLayer.getOutput();
    
}

void NNet::back(std::vector<float> desired_output)
{
    
    // clear errors form every layer
    inputLayer.clearErrors();
    
    for (int i = 0; i < hiddenLayers.size(); i++) {
        hiddenLayers[i].clearErrors();
    }
    
    outputLayer.clearErrors();
    
    
    // set and calculate new errors for every layer
    outputLayer.setError(desired_output); // deal output layer's error separately
    outputLayer.back();
    
    for(int i = (int)hiddenLayers.size() - 1; i >= 0 ; i-- )
        hiddenLayers[i].back();
    
    inputLayer.back();

}

void NNet::linkInput(int input_id,int hidden_id)
{
    hiddenLayers[0].neurons[hidden_id]->addLink(inputLayer.neurons[input_id]);
}

void NNet::linkHidden(int source_layer, int source_id, int dest_layer, int dest_id)
{
    
    if(hiddenLayers.size() > dest_layer &&
       hiddenLayers.size() > source_layer &&
       hiddenLayers[dest_layer].neurons.size() > dest_id &&
       hiddenLayers[source_layer].neurons.size() > source_id)
    {
        hiddenLayers[dest_layer].neurons[dest_id]->addLink(hiddenLayers[source_layer].neurons[source_id]);
    }
    else
    {
        std::cerr << "bad link indexes!!!\n";
    }
}

void NNet::link(linkData data)
{
    if(data.source_layer == 0)
        linkInput(data.source_id, data.dest_id);
    else
        linkHidden(data.source_layer, data.source_id, data.dest_layer, data.dest_id);
    
    nnData.lData.push_back(data);
    
}

void NNet::link(int source_layer, int source_id, int dest_layer, int dest_id)
{
    linkData data(source_layer,source_id,dest_layer,dest_id);
    link(data);
}

void NNet::saveNet(std::string filename)
{
    
    std::ofstream os(filename);
    
    if(os.is_open()) {
        std::vector<NeuronData> nData = inputLayer.getNeuronData();
    
        nnData.nData.insert(nnData.nData.end(), nData.begin(), nData.end());
    
        for(int i = 0; i < hiddenLayers.size(); i++) {
            nData = hiddenLayers[i].getNeuronData();
            nnData.nData.insert(nnData.nData.end(), nData.begin(), nData.end());
        }
    
        nData = outputLayer.getNeuronData();
        nnData.nData.insert(nnData.nData.end(), nData.begin(),nData.end());

        os << nnData.toString();
        os.close();

    } else
        std::cout << "no save file open";
    
}

void NNet::printSize()
{
    std::cout << "input Layer: " << inputLayer.neurons.size() <<
    " hidden layer: " << hiddenLayers.size() << " output layer:" << outputLayer.neurons.size() << "\n";

}
