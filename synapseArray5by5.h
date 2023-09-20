#ifndef _SYNAPSE_ARRAY_5BY5_H_
#define _SYNAPSE_ARRAY_5BY5_H_

#include <vector>
#include <math.h>
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
    int _standard[5][2]; // min, max
    int _mid[5][5];

    // input
    int _WL[5];

    // output
    int _ADCvalueN5[5];
    int _ADCvalueN6[5];
    int _ADCvalue[5];
    /*---------------------methods----------------*/

public:
    synapseArray5by5();
    ~synapseArray5by5();

    void setWLPulseWidth(int &WL0, int &WL1, int &WL2, int &WL3, int &WL4);
    void setADCvalueN5(int &ADC_0, int &ADC_1, int &ADC_2, int &ADC_3, int &ADC_4);
    void setADCvalueN6(int &ADC_0, int &ADC_1, int &ADC_2, int &ADC_3, int &ADC_4);
    void setADCvalue();
};
//**************************************************************************************************************//
#endif
