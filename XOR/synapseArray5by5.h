#ifndef _SYNAPSE_ARRAY_5BY5_H_
#define _SYNAPSE_ARRAY_5BY5_H_

#include <vector>
#include <math.h>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <fstream> //ofstream
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
    int _min[5][5];
    int _mid[5][5];
    int _max[5][5];

    // input
    int _WL[5];

    // output
    int _ADCvalueN5[5];
    int _ADCvalueN6[5];
    int _ADCvalueN5N6[5];

    // weight matrix
    int initialWeight[5][5];
    int targetWeight[5][5];
    int targetBias[5][5];

    int range;

    // (SGD) Stochastic Gradient Descent
    int p[5];
    int q[5];
    int P[5];
    int Q[5];
    /*---------------------methods----------------*/

public:
    synapseArray5by5();
    // synapseArray5by5(const std::string &initial_weight_path, const std::string &target_weight_path);
    ~synapseArray5by5();

    void setInitialWeight(const std::string &file_path);
    void setTargetWeight(const std::string &file_path);

    void setWLPulseWidth(int &WL0, int &WL1, int &WL2, int &WL3, int &WL4);
    void setADCvalueN5(int &ADC_0, int &ADC_1, int &ADC_2, int &ADC_3, int &ADC_4);
    void setADCvalueN6(int &ADC_0, int &ADC_1, int &ADC_2, int &ADC_3, int &ADC_4);
    void setADCvalueN5N6(int ADC_0, int ADC_1, int ADC_2, int ADC_3, int ADC_4);

    void setADCminValue(int rowNum);
    void setADCmidValue(int rowNum);
    void setADCmaxValue(int rowNum);
};
//**************************************************************************************************************//
#endif