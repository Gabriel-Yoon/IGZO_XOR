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
    // ADC values
    int _min[5][5];
    int _mid[5][5];
    int _max[5][5];
    int _gnd[5][5];
    int _noise[5][5];
    int _ref[5][5];

    // input (obsolete)
    int _WL[5];

    // ADC output saved by read_scaling
    int _ADCvalueTemp[5];
    int _ADCvalueN5[5];
    int _ADCvalueN6[5];
    int _ADCvalueN5N6[5];
    int _ADCbias[5];

    // weight matrix: w values in y = wx +b
    double _weight[5][5];
    double _weightADC[5][5];
    double _initialWeight[5][5];
    double _targetWeight[5][5];
    double _targetBias[5][5];

    // Gradient : Used in BackPropagation Update
    double _dW1[5][5];
    double _dW2[5][5];
    double _dH[5];
    double _dZ[5];

    // referencin range
    int _range;

    // (SGD) Stochastic Gradient Descent
    int p[5];
    int q[5];
    double P1[5];
    double Q1[5];
    double P2[5];
    double Q2[5];
    /*---------------------methods----------------*/

public:
    synapseArray5by5();
    // synapseArray5by5(const std::string &initial_weight_path, const std::string &target_weight_path);
    ~synapseArray5by5();

    void initialize();
    void setRange(int &range);
    void refresh();

    void setWLPulseWidth(int &WL0, int &WL1, int &WL2, int &WL3, int &WL4);
    void setADCvalueTemp(int ADC_0, int ADC_1, int ADC_2, int ADC_3, int ADC_4);
    void setADCvalueN5();
    void setADCvalueN6();
    void setADCvalueN5N6();

    void setADCminValue(int rowNum);
    void setADCmidValue(int rowNum);
    void setADCmaxValue(int rowNum);
    void setADCgndValue(int rowNum);
    void setADCnoiseValue(int rowNum);
    void setADCrefValue();
    void modifyADCrefValue();

    void setWeightToInitialWeight();
    void setInitialWeight();
    void setTargetWeight();
    void setTargetBias();

    void setdHfromADCvalue(double error);

    void clearQ1();
    void clearP1();
    void clearQ2();
    void clearP2();
};
//**************************************************************************************************************//
#endif