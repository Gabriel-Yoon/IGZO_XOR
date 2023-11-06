#ifndef _SYNAPSE_ARRAY_5BY5_H_
#define _SYNAPSE_ARRAY_5BY5_H_

#include <vector>
#include <math.h>
// #include <iostream>
// #include <sstream>
// #include <fstream> //ofstream
// #include <Arduino.h>

#define MAX 300
#define Bit_length 10

enum : int
{
    min,
    max
};

// Forward declarations.

//**************************************************************************************************************//
class synapseArray5by5
{
    /*---------------------fields-----------------*/
public:
    std::vector<std::vector<int>> _min;
    std::vector<std::vector<int>> _mid;
    std::vector<std::vector<int>> _max;
    std::vector<std::vector<int>> _gnd;

    // input
    std::vector<int> _WL;

    // output
    std::vector<int> _ADCvalueTemp;
    std::vector<int> _ADCvalueN5;
    std::vector<int> _ADCvalueN6;
    std::vector<int> _ADCvalueN5N6;

    // weight matrix
    std::vector<std::vector<double>> initialWeight;
    std::vector<std::vector<double>> targetWeight;
    std::vector<std::vector<double>> targetBias;

    int range;

    // (SGD) Stochastic Gradient Descent
    std::vector<int> p[5];
    std::vector<int> q[5];
    std::vector<int> P[5];
    std::vector<int> Q[5];
    /*---------------------methods----------------*/

public:
    synapseArray5by5();
    // synapseArray5by5(const std::string &initial_weight_path, const std::string &target_weight_path);
    ~synapseArray5by5();

    void setWLPulseWidth(int &WL0, int &WL1, int &WL2, int &WL3, int &WL4);
    void setADCvalueTemp(int ADC_0, int ADC_1, int ADC_2, int ADC_3, int ADC_4);
    void setADCvalueN5();
    void setADCvalueN6();
    void setADCvalueN5N6();

    void setADCminValue(int rowNum);
    void setADCmidValue(int rowNum);
    void setADCmaxValue(int rowNum);
    void setADCgndValue(int rowNum);

    void setInitialWeight();
    void setTargetWeight();
    void setTargetBias();
};
//**************************************************************************************************************//
#endif