#ifndef __libnn__libnnInterface__
#define __libnn__libnnInterface__

#include <stdio.h>
#include "libnn.h"
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include <string>

/*
 
 HiiriNNet:
 
 hiirenXY
 |
 v
 hiiriNNet
 |
 v
 liikeXY ja koonMuutos
 |
 v
 skaalaa (-1,1)
 |
 v
 muuta paikka & bound
 |
 v
 laske toivottu paikka
 |
 v
 NNet back()
 
 
 
 
 NNetin Katsellaan rutiini:
 
 inputit
 |
 v
 ryhmät? (vector<NNet>(groups_n,NNet(ValikoituInputJoukko, 1, kaikki_inputtien_yhdistelmä, ))  )
 |
 v
 tunneNNet (NNet(groups_n, 1, kaikki_groups_yhdistelmät, 8))
 |
 |
 v
 muodonLiike (vector<float>(8))
 |
 v
 skaalaa (-1,1)
 |
 v
 lisää MuodonArvoihin
 |
 v
 laske Desired output
 |
 v
 NNet back()
 
 Kuinka lasketaan DesireOut:
 
 - vertaa Inputtia Tilanteiden Inputtiin
 - valitse Inputteja lähin tilanteen output
 
 - laske outputin ja nykyisen muodon välinen eroavuus
 - skaalaaLuvut välille(0,1) ja boundaa
 
 
 
 */

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
    
    void TeeTilanne(std::vector<float> input, std::vector<float> output);
    
    void LaskeDesiredOut(std::vector<float> nykyinenPaikka);
    
    void Close();
}
#endif
